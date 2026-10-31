#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/ResourceCatalog.h"
#include "world/ViewDirection.h"

namespace game {

class MockAnimalSimulation;

class MockGameClient: public GameClient {
public:
    // Mundo con SOLO el jugador (compatibilidad con pruebas existentes).
    MockGameClient(int gridColumns, int gridRows, int startX, int startY,
                   int toolUseDurationTicks);

    // Mundo con el jugador + simulación de animales (wander). El catálogo
    // aporta el ritmo de paso de cada criatura (datos TOML); sin
    // apariencias de animales declaradas, corre sin animales (sin romper).
    MockGameClient(int gridColumns, int gridRows, int startX, int startY,
                   int toolUseDurationTicks,
                   const world::CharacterCatalog& characters);

    // El destructor se define en el .cpp: el unique_ptr de
    // MockAnimalSimulation necesita el tipo completo (está forward-declarado).
    ~MockGameClient() override;

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

    // contentId de un producto/ítem (mismo vocabulario que el inventario).
    std::string contentIdFor(world::ItemKind kind) const;

    // update() avanza la simulación un tick; worldState() solo consulta la vista.
    void advanceToolUse();

    // Emite el evento de obtención (si fue encolado por un Take) por un
    // solo tick. El servidor decidirá este evento; esto lo simula.
    void advanceObtained();

    void spawnAnimals(const world::CharacterCatalog& characters);

    // Toma lo que la simulación produjo en este tick y lo publica como
    // ítem del suelo (validando reglas del "server falso": tope y celda
    // ya ocupada por otro ítem).
    void collectProducedItems();

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

    // Productos de granja en el piso (huevos, leche): estado del
    // "server falso". Sin simulación queda vacío.
    std::vector<ViewDroppedItem> groundItems_;

    // Evento de obtención encolado por un Take exitoso: se emite en el
    // próximo update() y dura un solo tick.
    bool obtainedPending_ = false;
    world::ResourceKind obtainedKind_{};

    // La simulación de animales es del lado del "server falso": nula en
    // el mundo de solo jugador. Cada responsabilidad, en su clase.
    std::unique_ptr<MockAnimalSimulation> simulation_;

    LocalWorldView view_;
};

}  // namespace game
