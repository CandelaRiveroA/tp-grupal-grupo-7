#pragma once

#include <QApplication>
#include <memory>
#include <optional>

#include "common/common_socket.h"
#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

class LobbyWindow;

namespace game {
class GameClient;
}

namespace world {
class CharacterCatalog;
struct CharacterPalettes;
struct GridSpan;
}  // namespace world

namespace core {

class Application {
public:
    Application(int& argc, char** argv);

    int run();

private:
    // Carga el QSS embebido en el .qrc. Se llama al construir, cuando
    // QApplication ya existe.
    void loadFonts();
    void loadStyleSheet();

    // Paletas del editor de personaje (ojos, pelo y pantalón). Las usan
    // el lobby y el render del jugador en partida.
    world::CharacterPalettes loadPalettes() const;

    // Ventana Qt mínima. Devuelve nullopt si el jugador salió sin jugar.
    std::optional<SessionConfig> runLobby();

    // Carga los datos de la partida y corre la sesión de juego. Validar
    // que el lobby haya dado una sesión le corresponde a run().
    int runGame(SessionConfig session);

    std::unique_ptr<game::GameClient> createGameClient(Socket&& socket,
                                                       const world::CharacterCatalog& characters,
                                                       int toolUseDurationTicks) const;

    WindowConfig config_;
    QApplication app_;
};

}  // namespace core
