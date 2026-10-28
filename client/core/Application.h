#pragma once

#include <QApplication>
#include <memory>
#include <optional>

#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

class LobbyWindow;

namespace game {
class GameClient;
}

namespace world {
class Scenario;
class CharacterCatalog;
struct InventoryLayout;
struct ToolOverlayCatalog;
struct ResourceCatalog;
struct ObtainedBannerCatalog;
struct DigitCatalog;
}  // namespace world

namespace core {

class Application {
public:
    Application(int& argc, char** argv);

    int run();

private:
    // Carga el QSS embebido en el .qrc. Se llama al construir, cuando
    // QApplication ya existe.
    void loadStyleSheet();

    // Ventana Qt mínima. Devuelve nullopt si el jugador salió sin jugar.
    std::optional<SessionConfig> runLobby();

    int runGame(const SessionConfig& session);

    std::unique_ptr<game::GameClient> createGameClient() const;

    int runGameLoop(const SessionConfig& session, const world::Scenario& scenario,
                    game::GameClient& gameClient, const world::CharacterCatalog& characters,
                    const world::InventoryLayout& inventory,
                    const world::ToolOverlayCatalog& toolOverlay,
                    const world::ResourceCatalog& resources,
                    const world::ObtainedBannerCatalog& banner,
                    const world::DigitCatalog& digits);

    WindowConfig config_;
    QApplication app_;
};

}  // namespace core
