#pragma once

#include <cstdint>
#include <map>
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

private:
    struct RemotePlayer {
        int cellX = 0;
        int cellY = 0;
        world::Direction facing = world::Direction::Front;
        bool moving = false;
        int toolUseRemaining = 0;
        CharacterAppearance appearance;
    };

    void apply(const ServerMessage& message);
    // Aplica un mensaje al estado ya (sin pasar por el buffer de lote).
    void applyNow(const ServerMessage& message);

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

    std::map<uint8_t, RemotePlayer> players_;
    // IDs de los jugadores en la cama (último aviso de ServerSleepingPlayers).
    std::vector<uint8_t> sleepers_;
    std::map<uint8_t, ViewEntity> animals_;
    std::map<std::pair<int, int>, ViewResource> resources_;
    // Lista (no mapa por celda): varios huevos pueden estar apilados.
    std::vector<ViewDroppedItem> dropped_;
    std::map<int, ViewInventorySlot> inventory_;
    InteractionSession session_;
    // Lote Reset…SyncEnd en curso: se acumula y se aplica de una al llegar
    // el cierre. Si se aplicara de a mensajes, un frame entre el Reset y las
    // entidades dibujaría el mundo vacío (el parpadeo). Vacío = sin lote.
    std::vector<ServerMessage> batch_;
    // Eventos llegados desde el último update(); update() los pasa a view_.obtained.
    std::vector<ViewObtained> obtained_;
    LocalWorldView view_;
};

}  // namespace game
