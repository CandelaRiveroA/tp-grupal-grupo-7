#pragma once

#include <QApplication>

#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

#include <optional>

namespace qt {
class LobbyWindow;
}

namespace core {

// Orquestador del cliente. Secuencia:
//   1. runLobby(): ventanita Qt para crear/unirse a partida y elegir personaje.
//   2. Si el jugador confirmó, runGame(): el "main SDL", con todo el juego
//      (loop, input, render, config y multiplayer) adentro de SDL.
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

	// "main SDL": loop del juego. Corre hasta que se cierra la ventana.
	int runGame(const SessionConfig& session);

	WindowConfig config_;
	QApplication app_;
};

}
