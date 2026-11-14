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
#include "world/CharacterPaletteLoader.h"
#include "world/CharacterPalettes.h"
#include "world/ChatConfigLoader.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"
#include "world/CursorConfigLoader.h"
#include "world/DialogConfigLoader.h"
#include "world/HudOverlayConfigLoader.h"
#include "world/DigitCatalog.h"
#include "world/DigitConfigLoader.h"
#include "world/FontCatalog.h"
#include "world/Grid.h"
#include "world/InventoryConfigLoader.h"
#include "world/InventoryLayout.h"
#include "world/ObtainedBannerCatalog.h"
#include "world/ObtainedBannerConfigLoader.h"
#include "world/PromptLayout.h"
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
    // StardewValley: pergaminos; Pixel Operator: el resto de los textos.
    for (const char* path: {":/fonts/stardew.ttf", ":/fonts/pixel_operator.ttf"}) {
        if (QFontDatabase::addApplicationFont(path) < 0) {
            qWarning("No se pudo registrar la fuente %s", path);
        }
    }
}

world::CharacterPalettes Application::loadPalettes() const {
    QFile paletteFile(core::kPalettesResource);
    if (!paletteFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw world::ConfigError(world::ConfigErrorKind::UnreadableInput, core::kPalettesResource);
    }
    return world::CharacterPaletteLoader().loadFromString(paletteFile.readAll().toStdString());
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
    std::optional<SessionConfig> session;
    try {
        session = runLobby();
    } catch (const world::ConfigError& error) {
        const sdl::Logger logger;
        logger.error(error.what());
        return EXIT_FAILURE;
    }

    if (!session.has_value()) {
        return 0;
    }
    return runGame(std::move(*session));
}

std::optional<SessionConfig> Application::runLobby() {
    const world::CharacterCatalog characters = world::CharacterCatalogLoader().loadFromFile(
            core::assetPath(core::kCharactersFile));
    const world::CharacterPalettes palettes = loadPalettes();

    sdl::MusicPlayer music;
    LobbyWindow lobby(config_, characters, palettes, &music);
    lobby.show();
    app_.exec();

    return lobby.result();
}

int Application::runGame(SessionConfig session) {
    const sdl::Logger logger;
    if (!session.connection.has_value()) {
        logger.error("No hay conexion con el servidor.");
        return EXIT_FAILURE;
    }
    try {
        world::SceneDescription scene = world::SceneConfigLoader().loadFromFile(
                core::assetPath(core::kScenesFile), core::kInitialRoom);
        world::Scenario scenario(config_, std::move(scene));
        const world::CharacterCatalog characters = world::CharacterCatalogLoader().loadFromFile(
                core::assetPath(core::kCharactersFile));
        const world::CharacterPalettes palettes = loadPalettes();
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
                world::CursorConfigLoader().loadFromFile(core::assetPath(core::kCursorsFile));
        const world::DialogCatalog dialogs =
                world::DialogConfigLoader().loadFromFile(core::assetPath(core::kDialogsFile));
        const world::HudOverlayCatalog hudOverlays =
                world::HudOverlayConfigLoader().loadFromFile(
                        core::assetPath(core::kHudOverlaysFile));
        const world::HudOverlayCatalog lighting =
                world::HudOverlayConfigLoader().loadFromFile(
                        core::assetPath(core::kLightingFile));
        // La fuente de texto y el prompt del popup: datos puros cuya
        // traducción vive en su constructor (mismo ConfigError del
        // catch único de este borde).
        const world::FontCatalog font(core::assetPath(core::kTextFile));
        const world::PromptLayout prompt(core::assetPath(core::kTextFile));
        const world::ChatLayout chat =
                world::ChatConfigLoader().loadFromFile(core::assetPath(core::kChatFile));
        const int toolUseDuration =
                characters.dataFor(world::CreatureKind::Player).toolUseDurationTicks();
        std::unique_ptr<game::GameClient> gameClient =
                createGameClient(std::move(*session.connection), characters, toolUseDuration);
        PlaySession playSession(config_, scenario, *gameClient, characters, inventory, toolOverlay,
                                resources, banner, digits, cursors, dialogs, font, prompt, chat,
                                hudOverlays, lighting, palettes);
        playSession.run();
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
        Socket&& socket, const world::CharacterCatalog& characters,
        int toolUseDurationTicks) const {
    return std::make_unique<game::NetworkGameClient>(std::move(socket), toolUseDurationTicks,
                                                     characters);
}

}  // namespace core
