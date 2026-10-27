#pragma once

#include <QApplication>
#include <memory>
#include <optional>

#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

namespace qt {
class LobbyWindow;
}

namespace game {
class GameClient;
}

namespace world {
class Scenario;
struct GridSpan;
class CharacterCatalog;
struct InventoryLayout;
}  // namespace world

namespace core {

class Application {
public:
    Application(int& argc, char** argv);

    int run();

private:
    std::optional<SessionConfig> runLobby();

    int runGame(const SessionConfig& session);

    std::unique_ptr<game::GameClient> createGameClient() const;

    int runGameLoop(const SessionConfig& session, const world::Scenario& scenario,
                    game::GameClient& gameClient, const world::CharacterCatalog& characters,
                    const world::InventoryLayout& inventory);

    WindowConfig config_;
    QApplication app_;
};

}  // namespace core
