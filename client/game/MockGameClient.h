#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "world/ResourceCatalog.h"
#include "world/ViewDirection.h"

namespace game {

// "Server falso": simula el mundo del lado del servidor (jugador,
// minerales, inventario) sin red. Los animales NO viven acá: los
// superpone game::AnimalOverlayClient sobre cualquier GameClient.
class MockGameClient: public GameClient {
public:
    MockGameClient(int gridColumns, int gridRows, int startX, int startY,
                   int toolUseDurationTicks);

    void sendCommand(const protocol::ClientMessage& message) override;
    void update(int iteration) override;
    const LocalWorldView& worldState() const override;

private:
    // Mineral simulado: a los 3 golpes se agota. Simula al servidor.
    struct MineralSpot {
        world::ResourceKind kind;
        int cellX;
        int cellY;
        int hits = 0;
    };

    void walk(::Direction direction);

    // Celda que queda en esa dirección desde la posición del jugador.
    void frontCell(::Direction direction, int& cellX, int& cellY) const;

    void rebuildView();

    void rebuildInventory();

    void rebuildResources();

    void rebuildDropped();

    void placeMinerals();

    int mineralAt(int cellX, int cellY) const;

    int droppedAt(int cellX, int cellY) const;

    // Recoge el item dropeado en la celda del jugador (si lo hay),
    // programa el evento de obtención y suma el mineral al inventario.
    void collectAt(int cellX, int cellY);

    // Suma +1 al slot con ese contentId; si no existe, lo crea en el
    // primer slotIndex libre. Simula al servidor acumulando cantidad.
    void addToInventory(const std::string& contentId);

    // contentId estable de un recurso (mismo vocabulario que el servidor
    // y que inventory.toml / resources.toml).
    std::string contentIdFor(world::ResourceKind kind) const;

    // update() avanza la simulación un tick; worldState() solo consulta la vista.
    void advanceToolUse();

    // Emite el evento de obtención (si fue encolado por un Take) por un
    // solo tick. El servidor decidirá este evento; esto lo simula.
    void advanceObtained();

    int gridColumns_;
    int gridRows_;

    int playerCellX_;
    int playerCellY_;
    world::Direction playerFacing_;
    bool playerMoving_;

    int toolUseDurationTicks_;
    int toolUseRemaining_ = 0;

    // El golpe pendiente se confirma al completar la ventana (fin de la
    // animación), como hará el servidor.
    std::vector<MineralSpot> minerals_;
    int pendingMineral_ = -1;

    // Mineral caído al piso tras agotarse una roca. Recogible con Take.
    struct DroppedItem {
        world::ResourceKind kind;
        int cellX;
        int cellY;
    };
    std::vector<DroppedItem> dropped_;

    // Evento de obtención encolado por un Take exitoso: se emite en el
    // próximo update() y dura un solo tick.
    bool obtainedPending_ = false;
    world::ResourceKind obtainedKind_{};

    LocalWorldView view_;
};

}  // namespace game
