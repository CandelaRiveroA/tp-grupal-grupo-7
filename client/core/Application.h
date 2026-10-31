#pragma once

#include <QApplication>
#include <memory>
#include <optional>

#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

class LobbyWindow;

namespace game {
class GameClient;
class LocalWorldView;
class ViewMotion;
}

namespace sdl {
class Camera;
}

namespace world {
class Scenario;
class CharacterCatalog;
struct InventoryLayout;
struct ToolOverlayCatalog;
struct ResourceCatalog;
struct ObtainedBannerCatalog;
struct DigitCatalog;
struct SceneDescription;
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

    std::unique_ptr<game::GameClient> createGameClient(int toolUseDurationTicks) const;

    // Duración en ticks del golpe (UseTool) del jugador, derivada de los
    // datos del personaje (frames x ticks por cuadro). Es lo que el cliente
    // usa para mostrar la animación de golpe al recibir ToolUsed.
    int toolUseDurationTicks(const world::CharacterCatalog& characters) const;

    // La cámara sigue el rectángulo dibujado del jugador local, no su
    // celda lógica: así también se centra durante la interpolación del paso.
    void updateCamera(sdl::Camera& camera, const world::SceneDescription& scene,
                      const game::LocalWorldView& view,
                      const game::ViewMotion& motion, int iteration) const;

    int runGameLoop(const SessionConfig& session, const world::Scenario& scenario,
                    game::GameClient& gameClient, const world::CharacterCatalog& characters,
                    const world::InventoryLayout& inventory,
                    const world::ToolOverlayCatalog& toolOverlay,
                    const world::ResourceCatalog& resources,
                    const world::ObtainedBannerCatalog& banner, const world::DigitCatalog& digits);

    WindowConfig config_;
    QApplication app_;
};

}  // namespace core
