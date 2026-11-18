#include "game/PlayerController.h"

#include <algorithm>
#include <array>
#include <cstdlib>

#include "game/GameClient.h"
#include "game/GameConstants.h"

namespace game {

namespace {

// Celda alrededor del jugador -> dirección que la enfrenta. Tabla, no
// switch (convención §4): lo que no está acá está fuera de alcance.
struct FacingOffset {
    int dx;
    int dy;
    world::Direction direction;
};

constexpr std::array<FacingOffset, 4> kFacingOffsets = {{
        {0, -1, world::Direction::Back},
        {0, 1, world::Direction::Front},
        {-1, 0, world::Direction::Left},
        {1, 0, world::Direction::Right},
}};

}  // namespace

PlayerController::PlayerController(game::GameClient& client, const world::MoveCadence& cadence,
                                   int slotCount):
        client_(client), walkPacer_(cadence), slotCount_(slotCount) {
    walkHandlers_ = {
            {game::WalkIntent::None, []() {}},
            {game::WalkIntent::Step, [this]() { stepWalk(); }},
            {game::WalkIntent::Stop, []() {}},
    };
    commandHandlers_ = {
            {game::InputCommand::Kind::UseTool,
             [this](const game::InputCommand&, const std::optional<world::Direction>& pressed) {
                 handleUseTool(pressed);
             }},
            {game::InputCommand::Kind::Collect,
             [this](const game::InputCommand&, const std::optional<world::Direction>& pressed) {
                 handleCollect(pressed);
             }},
            {game::InputCommand::Kind::SelectSlot,
             [this](const game::InputCommand& command, const std::optional<world::Direction>&) {
                 handleSelectSlot(command);
             }},
    };
}

PlayerEvents PlayerController::tick(int iteration, const std::optional<world::Direction>& pressed,
                                    const std::vector<game::InputCommand>& commands) {
    events_ = PlayerEvents{};
    if (pressed.has_value()) {
        // El movimiento manual manda: cancela la auto-caminata.
        walkTarget_.reset();
    }
    walkHandlers_.at(walkPacer_.intentFor(iteration, walkingDirection(pressed)))();
    for (const game::InputCommand& command: commands) {
        commandHandlers_.at(command.kind)(command, pressed);
    }
    return events_;
}

int PlayerController::activeSlot() const { return inventorySelector_.active(); }

void PlayerController::interact() {
    client_.sendCommand(moveCommand_.take(walkPacer_.lastDirection(), activeSlot()));
}

void PlayerController::useToward(const world::GridCell& target,
                                 const std::optional<world::Direction>& pressed) {
    if (pressed.has_value()) {
        // No se usa herramienta mientras se camina (como la acción de
        // input "usar").
        return;
    }
    const LocalWorldView& state = client_.worldState();
    if (state.entities.empty()) {
        return;
    }
    const ViewEntity& local = state.entities[kLocalPlayerIndex];
    const int dx = target.column - local.cellX;
    const int dy = target.row - local.cellY;
    if (dx == 0 && dy == 0) {
        // El huevo bajo el jugador queda tapado por su sprite: el click va como Take.
        const auto onOwnCell = [&](const ViewDroppedItem& item) {
            return item.interactive && item.cellX == local.cellX && item.cellY == local.cellY;
        };
        if (std::any_of(state.droppedItems.begin(), state.droppedItems.end(), onOwnCell)) {
            interact();
            return;
        }
        // La celda propia: la herramienta apunta a donde mira.
        useTool(walkPacer_.lastDirection());
        return;
    }
    for (const FacingOffset& offset: kFacingOffsets) {
        if (offset.dx == dx && offset.dy == dy) {
            useTool(offset.direction);
            return;
        }
    }
    // Fuera de las celdas alrededor: el click no dispara nada (lo
    // válido es apuntar donde se golpea; el servidor validará la celda
    // exacta cuando el protocolo la lleve).
}

void PlayerController::stepWalk() {
    client_.sendCommand(moveCommand_.propose(walkPacer_.lastDirection()));
}

void PlayerController::walkTo(const world::GridCell& target) {
    walkTarget_ = target;
}

std::optional<world::Direction> PlayerController::walkingDirection(
        const std::optional<world::Direction>& pressed) {
    if (pressed.has_value() || !walkTarget_.has_value()) {
        return pressed;
    }
    const LocalWorldView& state = client_.worldState();
    if (state.entities.empty()) {
        walkTarget_.reset();
        return std::nullopt;
    }
    const ViewEntity& local = state.entities[kLocalPlayerIndex];
    const int dx = walkTarget_->column - local.cellX;
    const int dy = walkTarget_->row - local.cellY;
    if (dx == 0 && dy == 0) {
        // Llegó: se terminó la auto-caminata.
        walkTarget_.reset();
        return std::nullopt;
    }
    // Un paso por eje, priorizando el que más falta (el ritmo lo sigue
    // poniendo el pacer; esto solo elige hacia dónde).
    if (dx != 0 && (dy == 0 || std::abs(dx) >= std::abs(dy))) {
        return dx > 0 ? world::Direction::Right : world::Direction::Left;
    }
    return dy > 0 ? world::Direction::Front : world::Direction::Back;
}

void PlayerController::useTool(const world::Direction direction) {
    // El slot activo viaja con el Pick: el servidor usa esa herramienta
    // (y decide si se puede).
    client_.sendCommand(moveCommand_.use(direction, activeSlot()));
}

void PlayerController::handleUseTool(const std::optional<world::Direction>& pressed) {
    if (!pressed.has_value()) {
        useTool(walkPacer_.lastDirection());
    }
}

void PlayerController::handleCollect(const std::optional<world::Direction>& pressed) {
    if (!pressed.has_value()) {
        client_.sendCommand(moveCommand_.take(walkPacer_.lastDirection(), activeSlot()));
    }
}

void PlayerController::handleSelectSlot(const game::InputCommand& command) {
    const int previous = inventorySelector_.active();
    inventorySelector_.select(command.slot, slotCount_);
    if (inventorySelector_.active() != previous) {
        events_.toolSwitched = true;
    }
}

}  // namespace game
