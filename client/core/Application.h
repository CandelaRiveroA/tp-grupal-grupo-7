#pragma once

#include <QApplication>

#include "core/SessionConfig.h"
#include "core/WindowConfig.h"

#include <memory>
#include <optional>

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
struct ToolOverlayCatalog;
struct ResourceCatalog;
}

namespace core {

class Application {
public:
	Application(int& argc, char** argv);

	int run();

private:
	std::optional<SessionConfig> runLobby();

	int runGame(const SessionConfig& session);

	std::unique_ptr<game::GameClient> createGameClient(
		const world::GridSpan& grid, int toolUseDurationTicks) const;

	// Duración en ticks del golpe (UseTool) del jugador, derivada de los
	// datos del personaje (frames x ticks por cuadro). Es lo que el mock
	// usa para simular cuánto reporta UseTool, como lo haría el servidor.
	int toolUseDurationTicks(
		const world::CharacterCatalog& characters) const;

	int runGameLoop(const SessionConfig& session,
			const world::Scenario& scenario,
			game::GameClient& gameClient,
			const world::CharacterCatalog& characters,
			const world::InventoryLayout& inventory,
			const world::ToolOverlayCatalog& toolOverlay,
			const world::ResourceCatalog& resources);

	WindowConfig config_;
	QApplication app_;
};

}
