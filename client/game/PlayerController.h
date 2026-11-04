#pragma once

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "game/InputCommand.h"
#include "game/InventorySelector.h"
#include "game/MoveCommand.h"
#include "game/WalkPacer.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ViewDirection.h"

namespace game {

class GameClient;

// Eventos del jugador que la capa de presentación traduce a efectos
// (sonidos). DTO plano.
struct PlayerEvents {
    bool toolSwitched = false;
};

// Dueño de la intención del jugador local: traduce el input (dirección
// pulsada + comandos one-shot) en comandos hacia la fuente de estado y en
// la selección de slot del inventario. No conoce SDL: recibe datos planos.
class PlayerController {
public:
    // slotCount: cuántos slots tiene el inventario (para validar la
    // selección). Lo pasa quien construye desde los datos del layout
    // (InventoryLayout::count).
    PlayerController(game::GameClient& client, const world::MoveCadence& cadence,
                     int slotCount);

    // Procesa un tick: primero el movimiento sostenido (un paso por
    // cadencia) y después las intenciones one-shot en orden de pulsación.
    PlayerEvents tick(int iteration, const std::optional<world::Direction>& pressed,
                      const std::vector<game::InputCommand>& commands);

    // Intención de tocar/agarrar lo que se clickeó: manda un Take hacia
    // la fuente de estado, que decide (cercanía, validez, qué pasa).
    void interact();

    // Intención de USAR la herramienta activa hacia la celda clickeada
    // (el click en el piso es el disparador de la herramienta). Solo si
    // la celda está alrededor del jugador (la propia o una adyacente en
    // un eje): lo válido es apuntar donde se golpea. La dirección de la
    // intención la resuelve acá; la fuente de estado decide si se puede
    // (mañana el servidor validará la celda exacta).
    void useToward(const world::GridCell& target,
                   const std::optional<world::Direction>& pressed);

    int activeSlot() const;

private:
    void stepWalk();
    void handleUseTool(const std::optional<world::Direction>& pressed);
    void handleCollect(const std::optional<world::Direction>& pressed);
    void handleSelectSlot(const game::InputCommand& command);

    // Usa la herramienta activa (el slot seleccionado) hacia una
    // dirección: el camino común del click y de la acción de input
    // "usar" (rebindeable). La fuente decide el resto.
    void useTool(world::Direction direction);

    using WalkHandler = std::function<void()>;
    using CommandHandler =
            std::function<void(const game::InputCommand&,
                               const std::optional<world::Direction>&)>;

    // Despacho por tabla, inicializada en el ctor y consultada con .at():
    // agregar un caso nuevo es sumar una entrada, no tocar el despacho.
    // .at() lanza si llega un valor desconocido.
    std::unordered_map<game::WalkIntent, WalkHandler> walkHandlers_;
    std::unordered_map<game::InputCommand::Kind, CommandHandler> commandHandlers_;

    game::GameClient& client_;
    MoveCommand moveCommand_;
    WalkPacer walkPacer_;
    InventorySelector inventorySelector_;
    int slotCount_;
    PlayerEvents events_;
};

}  // namespace game
