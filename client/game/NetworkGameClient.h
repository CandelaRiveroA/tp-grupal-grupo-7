#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "common/common_command.h"
#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "common/common_queue.h"
#include "common/common_server_status.h"
#include "common/common_socket.h"
#include "game/GameClient.h"
#include "game/InteractionSession.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/ContentIds.h"

#include "client_receiver.h"
#include "client_sender.h"

namespace game {

class NetworkGameClient: public GameClient {
public:
    // toolUseDurationTicks: cuánto dura la animación de golpe en el cliente.
    // Es presentación: el servidor avisa cuándo se usó la herramienta.
    // characters: duración de la animación de ordeñe (presentación).
    NetworkGameClient(Socket&& socket, int toolUseDurationTicks,
                      const world::CharacterCatalog& characters);
    ~NetworkGameClient() override;

    void sendCommand(const protocol::ClientMessage& message) override;
    void update(int iteration) override;
    const LocalWorldView& worldState() const override;
    bool connected() const override;

    
    void sendChat(const std::string& text) override;
    void receiveChat(std::uint8_t playerId, const std::string& characterName,
                     const std::string& text) override;

private:
    struct RemotePlayer {
        int cellX = 0;
        int cellY = 0;
        world::Direction facing = world::Direction::Front;
        bool moving = false;
        int toolUseRemaining = 0;
        // Herramienta del último golpe (la manda el servidor en ServerToolUsed).
        std::optional<world::ItemKind> activeTool;
        CharacterAppearance appearance;
        int health = 0;
        int maxHealth = 0;
        int energy = 0;      // x100
        int maxEnergy = 0;  // x100
        bool fainted = false;  // desmayado por falta de energía
        // Reacción al daño: la enciende el flanco de health en
        // ServerStatus (bajó = titilación con ventana; 0 = desmayo, se
        // queda tirado hasta que la vida vuelva a subir). Mismo dato y
        // misma regla que World::damage_player en el server.
        world::ActionKind reaction = world::ActionKind::Idle;
        int reactionRemaining = 0;
    };

    // Enemigo del server: la vista lo sigue por id; la acción dura su ventana.
    struct RemoteEnemy {
        ViewEntity entity;
        // Última acción recibida: detecta el flanco que enciende la ventana.
        EnemyAction lastReceived = EnemyAction::Idle;
        // Ticks que faltan para que el golpe o el daño vuelvan a quieto.
        int actionRemaining = 0;
    };

    void apply(const ServerMessage& message);
    // Aplica un mensaje al estado ya (sin pasar por el buffer de lote).
    void applyNow(const ServerMessage& message);
    // El lote es el estado completo: enemigos y proyectiles que no volvieron
    // en él ya no existen en el server (una llama que llegó a su destino o un
    // cadáver viejo tras regenerar).
    void pruneToBatch();

    // Estado de daño del protocolo -> estado visible de la vista. Lo que
    // no llega a ser el golpe final cae en Intact (seguro ante wire roto).
    world::ResourceState resourceStateFor(ResourceDamage damage) const;

    // Contenido equipado (inventory.toml) -> herramienta. Sin fila =
    // nullopt (manos o producto).
    std::optional<world::ItemKind> toolKindFor(const std::string& contentId) const;

    // Vocabulario compartido con el servidor (characters.toml).
    std::optional<world::CreatureKind> creatureKindFor(const std::string& kind) const;

    // Un Take cerca de una vaca lista abre la animación de ordeñe; la leche la
    // da el servidor.
    void startMilking();

    // Un Take con un huevo en las 8 vecinas abre la animación de recogida, como
    // un cultivo; el item lo entrega el servidor.
    void startPickup();

    Socket socket_;
    ProtocolSender sender_;
    ProtocolReceiver receiver_;
    Queue<protocol::ClientMessage> commands_;
    Queue<ServerMessage> messages_;
    ClientSender clientSender_;
    ClientReceiver clientReceiver_;

    uint8_t localPlayerId_;
    int toolUseDurationTicks_;
    const world::CharacterCatalog* characters_;
    // Tabla de traducción protocolo -> vista (convención §4: diccionario
    // miembro, inicializado en el constructor).
    std::map<ResourceDamage, world::ResourceState> resourceStates_;
    // Vocabulario contentId <-> ItemKind (convención §2: sin función
    // libre ni static; vive como miembro).
    world::ContentIds contentIds_;
    // Vocabulario kind del servidor -> CreatureKind (convención §4:
    // diccionario miembro, inicializado en el constructor).
    std::map<std::string, world::CreatureKind> creatureKinds_;
    std::map<ServerMarketSide, world::MarketSide> marketSides_;

    std::map<uint8_t, RemotePlayer> players_;
    // IDs de los jugadores en la cama (último aviso de ServerSleepingPlayers).
    std::vector<uint8_t> sleepers_;
    std::map<uint8_t, ViewEntity> animals_;
    std::map<uint16_t, RemoteEnemy> enemies_;
    // Proyectiles (llamas del skull): sin ventana de acción, solo la celda.
    std::map<uint16_t, ViewEntity> projectiles_;
    std::map<std::pair<int, int>, ViewResource> resources_;
    // Lista (no mapa por celda): varios huevos pueden estar apilados.
    std::vector<ViewDroppedItem> dropped_;
    std::vector<ViewCrop> crops_;
    std::vector<ViewFurniture> furniture_;
    std::vector<ViewGroundEffect> groundEffects_;
    std::map<int, ViewInventorySlot> inventory_;
    InteractionSession session_;
    // Lote Reset…SyncEnd en curso: se acumula y se aplica de una al llegar
    // el cierre. Si se aplicara de a mensajes, un frame entre el Reset y las
    // entidades dibujaría el mundo vacío (el parpadeo). Vacío = sin lote.
    std::vector<ServerMessage> batch_;
    // Eventos llegados desde el último update(); update() los pasa a view_.obtained.
    std::vector<ViewObtained> obtained_;
    // Pedido de regalo desde el último update(); update() lo pasa a view_.giftRequest.
    std::optional<ViewGiftRequest> giftRequest_;
    LocalWorldView view_;
};

}  // namespace game
