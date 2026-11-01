#include "Application.h"

#include <QFile>
#include <QFontDatabase>
#include <QString>
#include <QtGlobal>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "core/Constants.h"
#include "core/PlaySession.h"
#include "game/GameClient.h"
#include "game/NetworkGameClient.h"
#include "qt/LobbyWindow.h"
#include "sdl/audio/MusicPlayer.h"
#include "sdl/log/Logger.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"
#include "world/DigitCatalog.h"
#include "world/DigitConfigLoader.h"
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
    loadFonts();
    loadStyleSheet();
}

void Application::loadFonts() {
    const int id = QFontDatabase::addApplicationFont(":/fonts/stardew.ttf");
    if (id < 0) {
        qWarning("No se pudo registrar la fuente :/fonts/stardew.ttf");
    }
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

    sdl::MusicPlayer music;
    LobbyWindow lobby(config_, &music);
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
        const int toolUseDuration =
                characters.dataFor(world::CreatureKind::Player).toolUseDurationTicks();
        std::unique_ptr<game::GameClient> gameClient = createGameClient(toolUseDuration);
        PlaySession session(config_, scenario, *gameClient, characters, inventory,
                            toolOverlay, resources, banner, digits);
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

std::unique_ptr<game::GameClient> Application::createGameClient(int toolUseDurationTicks) const {
    return std::make_unique<game::NetworkGameClient>(core::kServerHost, core::kServerPort,
                                                     toolUseDurationTicks);
}

}  // namespace core
