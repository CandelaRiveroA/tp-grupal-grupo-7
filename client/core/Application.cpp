#include "Application.h"

#include <optional>

#include "qt/LobbyWindow.h"
#include "sdl/GameLoop.h"
#include "sdl/InputManager.h"
#include "sdl/Renderer.h"
#include "sdl/SdlContext.h"
#include "world/Player.h"
#include "world/Scenario.h"

namespace core {

namespace {

constexpr int kPlayerSize = 32;
constexpr int kPlayerSpeed = 5;
}

Application::Application(int& argc, char** argv)
	: config_(640, 480, "TP Final — Grupo 7")
	, app_(argc, argv) {}

int Application::run() {

	std::optional<SessionConfig> session = runLobby();
	if (!session.has_value()) {
		return 0;
	}

	// Delegación Qt → SDL: acá empieza el "main SDL".
	return runGame(*session);
}

std::optional<SessionConfig> Application::runLobby() {

	qt::LobbyWindow lobby(config_);
	lobby.show();
	app_.exec();

	if (!lobby.confirmed()) {
		return std::nullopt;
	}
	return lobby.session();
}

int Application::runGame(const SessionConfig& session) {

	(void)session;

	world::Player player((config_.width() - kPlayerSize) / 2,
			     (config_.height() - kPlayerSize) / 2,
			     kPlayerSize, kPlayerSize, kPlayerSpeed);
	world::Scenario scenario(config_, player);

	sdl::SdlContext sdlContext(config_);
	sdl::Renderer renderer(sdlContext);
	sdl::InputManager input;

	sdl::GameLoop loop(input, [&]() {

		// Flujo de movimiento SDL: la tecla se detecta acá. Hoy la
		// validación es local (clamp contra los bordes); cuando el
		// server mande el status de "¿me puedo mover?" se consultará
		// antes de aplicar el move.
		int dx = 0;
		int dy = 0;
		if (input.left()) dx -= player.speed();
		if (input.right()) dx += player.speed();
		if (input.up()) dy -= player.speed();
		if (input.down()) dy += player.speed();
		if (dx != 0 || dy != 0) {
			player.move(dx, dy, scenario.width(), scenario.height());
		}

		renderer.render(scenario);
	});

	loop.run();
	return 0;
}

}
