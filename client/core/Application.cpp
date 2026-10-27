#include "Application.h"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>

#include "common/common_command.h"
#include "core/Constants.h"
#include "core/StartupScreens.h"
#include "core/StartupSequence.h"
#include "game/ActionResolver.h"
#include "game/FootstepCue.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "game/MoveCommand.h"
#include "game/ToolUse.h"
#include "game/ViewMotion.h"
#include "game/WalkPacer.h"
#include "qt/LobbyWindow.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/loop/GameLoop.h"
#include "sdl/input/InputManager.h"
#include "sdl/log/Logger.h"
#include "sdl/render/Renderer.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/context/SdlContext.h"
#include "world/CreatureCatalog.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/ConfigError.h"
#include "world/EntityAppearance.h"
#include "world/InventoryConfigLoader.h"
#include "world/InventoryLayout.h"
#include "world/Scenario.h"
#include "world/SceneConfigLoader.h"
#include "world/SceneDescription.h"

namespace core {

Application::Application(int& argc, char** argv)
	: config_(kWindowWidth, kWindowHeight, kWindowTitle)
	, app_(argc, argv) {}

int Application::run() {

	std::optional<SessionConfig> session = runLobby();
	if (!session.has_value()) {
		return 0;
	}

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
	const sdl::Logger logger;
	try {
		world::Scenario scenario(config_,
					 core::assetPath(core::kScenesFile),
					 core::kInitialRoom);
		const world::CharacterCatalog characters =
			world::CharacterCatalogLoader().loadFromFile(
				core::assetPath(core::kCharactersFile));
		const world::InventoryLayout inventory =
			world::InventoryConfigLoader().loadFromFile(
				core::assetPath(core::kInventoryFile));
		const world::GridSpan& grid = scenario.scene().grid;
		std::unique_ptr<game::GameClient> gameClient =
			createGameClient(grid);
		return runGameLoop(session, scenario, *gameClient, characters,
				   inventory);
	} catch (const world::ConfigError& error) {
		logger.error(error.what());
		return EXIT_FAILURE;
	} catch (const std::exception& error) {
		logger.error(error.what());
		return EXIT_FAILURE;
	}
}

std::unique_ptr<game::GameClient> Application::createGameClient(
	const world::GridSpan& grid) const {
	return std::make_unique<game::MockGameClient>(
		grid.columns, grid.rows, grid.columns / 2, grid.rows / 2);
}

int Application::runGameLoop(const SessionConfig& session,
			     const world::Scenario& scenario,
			     game::GameClient& gameClient,
			     const world::CharacterCatalog& characters,
			     const world::InventoryLayout& inventory) {

	(void)session;

	sdl::SdlContext sdlContext(config_);
	sdl::Renderer renderer(sdlContext, characters);
	sdl::InventoryHud inventoryHud(sdlContext, inventory);
	sdl::InputManager input;
	sdl::AudioPlayer audio;

	const StartupScreens screens(sdlContext, renderer, audio, scenario);
	const game::MoveCommand moveCommand;

	game::ViewMotion motion(characters);

	game::FootstepCue footstepCue;

	const world::CreatureData& localCreature =
		characters.dataFor(world::CreatureKind::Player);
	game::WalkPacer walkPacer(localCreature.cadence);

	const auto useToolAction = [](const world::CreatureData& c)
		-> const world::ActionData* {
		for (const world::DirectionAppearance& dir : *c.directions) {
			const auto it =
				dir.actions->find(world::ActionKind::UseTool);
			if (it != dir.actions->end()) {
				return &it->second;
			}
		}
		return nullptr;
	};
	const world::ActionData* const useTool = useToolAction(localCreature);
	game::ToolUse toolUse = useTool != nullptr ? game::ToolUse(*useTool)
						    : game::ToolUse(1);

	constexpr std::size_t kPlayerIndex = 0;
	const game::ActionResolver actionResolver(motion, toolUse,
						  kPlayerIndex);

	sdl::GameLoop loop(input, [&](int it) {


		const game::LocalWorldView& state = gameClient.worldState();

		const GamePhase phase = phaseForIteration(it);
		if (screens.draw(phase, it, state, motion, actionResolver)) {
			return;
		}

		std::optional<protocol::Direction> pressed;
		if (input.left()) {
			pressed = protocol::Direction::Left;
		} else if (input.right()) {
			pressed = protocol::Direction::Right;
		} else if (input.up()) {
			pressed = protocol::Direction::Back;
		} else if (input.down()) {
			pressed = protocol::Direction::Front;
		}

		switch (walkPacer.intentFor(it, pressed)) {
		case game::WalkIntent::None:
			break;
		case game::WalkIntent::Step: {
			gameClient.sendCommand(moveCommand.propose(
				state, walkPacer.lastDirection()));
			break;
		}
		case game::WalkIntent::Stop:
			gameClient.sendCommand(moveCommand.hold(
				state, walkPacer.lastDirection()));
			break;
		}

		if (input.takeUseTool() && !pressed.has_value()) {
			gameClient.sendCommand(
				moveCommand.use(state, walkPacer.lastDirection()));
			toolUse.trigger(it);
		}

		const game::LocalWorldView& drawing = gameClient.worldState();
		motion.observe(drawing, it);

		if (!drawing.entities.empty()) {
			const game::ViewEntity& player =
				drawing.entities[kPlayerIndex];
			const world::CreatureData& creature =
				characters.dataFor(player.kind);
			const int walkFrame = world::frameIndexFor(
				creature, player.facing,
				world::ActionKind::Walk,
				motion.animationTickFor(kPlayerIndex, it));
			const bool walking = motion.isWalking(
				kPlayerIndex, it, player.moving);
			if (footstepCue.update(walking, walkFrame)) {
				audio.play(sdl::SoundId::Step);
			}
		}

		renderer.renderScene(scenario.scene(), drawing, motion,
				     actionResolver, it);
		inventoryHud.draw(drawing.inventory);
		sdlContext.present();
	});

	loop.run();
	return 0;
}

}
