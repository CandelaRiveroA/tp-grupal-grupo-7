#pragma once

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"

namespace game {

class MockGameClient: public GameClient {
public:
    MockGameClient(int gridColumns, int gridRows, int startX, int startY);

    void sendCommand(const protocol::ClientMessage& message) override;
    const LocalWorldView& worldState() const override;

private:
    void walk(::Direction direction);

    void rebuildView();

    void rebuildInventory();

    int gridColumns_;
    int gridRows_;

    int playerCellX_;
    int playerCellY_;
    world::Direction playerFacing_;
    bool playerMoving_;

    LocalWorldView view_;
};

}  // namespace game
