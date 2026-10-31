#include "game/PlayerController.h"

#include "game/GameClient.h"

namespace game {

PlayerController::PlayerController(game::GameClient& client,
                                   const world::MoveCadence& cadence, int slotCount)
        : client_(client), walkPacer_(cadence), slotCount_(slotCount) {
    walkHandlers_ = {
            {game::WalkIntent::None, []() {}},
            {game::WalkIntent::Step, [this]() { stepWalk(); }},
            {game::WalkIntent::Stop, []() {}},
    };
    commandHandlers_ = {
            {game::InputCommand::Kind::UseTool,
             [this](const game::InputCommand&,
                    const std::optional<world::Direction>& pressed) {
                 handleUseTool(pressed);
             }},
            {game::InputCommand::Kind::Collect,
             [this](const game::InputCommand&,
                    const std::optional<world::Direction>& pressed) {
                 handleCollect(pressed);
             }},
            {game::InputCommand::Kind::SelectSlot,
             [this](const game::InputCommand& command,
                    const std::optional<world::Direction>&) {
                 handleSelectSlot(command);
             }},
    };
}

PlayerEvents PlayerController::tick(
        int iteration, const std::optional<world::Direction>& pressed,
        const std::vector<game::InputCommand>& commands) {
    events_ = PlayerEvents{};
    walkHandlers_.at(walkPacer_.intentFor(iteration, pressed))();
    for (const game::InputCommand& command : commands) {
        commandHandlers_.at(command.kind)(command, pressed);
    }
    return events_;
}

int PlayerController::activeSlot() const {
    return inventorySelector_.active();
}

void PlayerController::stepWalk() {
    client_.sendCommand(moveCommand_.propose(walkPacer_.lastDirection()));
}

void PlayerController::handleUseTool(const std::optional<world::Direction>& pressed) {
    if (!pressed.has_value()) {
        client_.sendCommand(moveCommand_.use(walkPacer_.lastDirection()));
    }
}

void PlayerController::handleCollect(const std::optional<world::Direction>& pressed) {
    if (!pressed.has_value()) {
        client_.sendCommand(moveCommand_.take(walkPacer_.lastDirection()));
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
