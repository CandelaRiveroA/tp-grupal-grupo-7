#include "Application.h"

#include <QFile>
#include <QString>
#include <QtGlobal>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "core/Constants.h"
#include "core/PlaySession.h"
#include "game/MockWorldClient.h"
#include "game/GameClient.h"
#include "game/NetworkGameClient.h"
#include "qt/LobbyWindow.h"
#include "sdl/log/Logger.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"
#include "world/CursorConfigLoader.h"
#include "world/DialogConfigLoader.h"
#include "world/DigitCatalog.h"
#include "world/DigitConfigLoader.h"
#include "world/Grid.h"
#include "world/InventoryConfigLoader.h"
#include "world/InventoryLayout.h"
#include "world/ObtainedBannerCatalog.h"
#include "world/ObtainedBannerConfigLoader.h"
#include "world/ResourceCatalog.h"
#include "world/ResourceConfigLoader.h"
#include "world/Scenario.h"
#include "world/SceneConfigLoader.h"
#include "world/SceneDescription.h"
#include "world/ToolOverlay.h"
#include "world/ToolOverlayLoader.h"

namespace core {

Application::Application(int& argc, char** argv):
        config_(kWindowWidth, kWindowHeight, kWindowTitle), app_(argc, argv) {
    loadStyleSheet();
}

void Application::loadStyleSheet() {
    QFile file(":/ui.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("No se pudo abrir el recurso :/ui.qss");
        return;
    }
    app_.setStyleSheet(QString::fromUtf8(file.readAll()));
}

int Application::run() {

    std::optional<SessionConfig> session = runLobby();
    if (!session.has_value()) {
        return 0;
    }

    return runGame();
}

std::optional<SessionConfig> Application::runLobby() {

    LobbyWindow lobby(config_);
    lobby.show();
    app_.exec();

    return lobby.result();
}

int Application::runGame() {
    const sdl::Logger logger;
    try {
        world::SceneDescription scene = world::SceneConfigLoader().loadFromFile(
                core::assetPath(core::kScenesFile), core::kInitialRoom);
        world::Scenario scenario(config_, std::move(scene));
        const world::CharacterCatalog characters = world::CharacterCatalogLoader().loadFromFile(
                core::assetPath(core::kCharactersFile));
        const world::InventoryLayout inventory =
                world::InventoryConfigLoader().loadFromFile(core::assetPath(core::kInventoryFile));
        const world::ToolOverlayCatalog toolOverlay =
                world::ToolOverlayLoader().loadFromFile(core::assetPath(core::kToolsOverlayFile));
        const world::ResourceCatalog resources =
                world::ResourceConfigLoader().loadFromFile(core::assetPath(core::kResourcesFile));
        const world::ObtainedBannerCatalog banner =
                world::ObtainedBannerConfigLoader().loadFromFile(
                        core::assetPath(core::kObtainedBannerFile));
        const world::DigitCatalog digits =
                world::DigitConfigLoader().loadFromFile(core::assetPath(core::kDigitsFile));
        const world::CursorCatalog cursors =
                world::CursorConfigLoader().loadFromFile(
                        core::assetPath(core::kCursorsFile));
        const world::DialogCatalog dialogs =
                world::DialogConfigLoader().loadFromFile(
                        core::assetPath(core::kDialogsFile));
        const int toolUseDuration =
                characters.dataFor(world::CreatureKind::Player).toolUseDurationTicks();
        const world::GridSpan& grid = scenario.scene().grid;
        std::unique_ptr<game::GameClient> gameClient =
                createGameClient(grid, characters, toolUseDuration);
        PlaySession session(config_, scenario, *gameClient, characters, inventory,
                            toolOverlay, resources, banner, digits, cursors,
                            dialogs);
        session.run();
        return 0;
    } catch (const world::ConfigError& error) {
        logger.error(error.what());
        return EXIT_FAILURE;
    } catch (const std::exception& error) {
        logger.error(error.what());
        return EXIT_FAILURE;
    }
}

std::unique_ptr<game::GameClient> Application::createGameClient(
        const world::GridSpan& grid,
        const world::CharacterCatalog& characters,
        int toolUseDurationTicks) const {
    // Fuente de estado: el server por red (contrato §6: al conectar la
    // red, el único cambio es quién llena la vista). La capa de
    // animales se superpone hoy en el cliente; cuando el server los
    // tenga, se saca y nada más cambia esta construcción.
    return std::make_unique<game::MockWorldClient>(
            std::make_unique<game::NetworkGameClient>(
                    core::kServerHost, core::kServerPort,
                    toolUseDurationTicks),
            characters, grid);
}

}  // namespace core
