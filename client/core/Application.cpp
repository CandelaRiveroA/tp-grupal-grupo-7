#include "Application.h"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "common/common_command.h"
#include "core/Constants.h"
#include "core/StartupScreens.h"
#include "core/StartupSequence.h"
#include "game/ActionResolver.h"
#include "game/FootstepCue.h"
#include "game/ResourceAudioCue.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "game/MoveCommand.h"
#include "game/ViewMotion.h"
#include "game/WalkPacer.h"
#include "game/InventorySelector.h"
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
#include "world/ToolOverlay.h"
#include "world/ToolOverlayLoader.h"
#include "world/ResourceCatalog.h"
#include "world/ResourceConfigLoader.h"

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
		world::SceneDescription scene =
			world::SceneConfigLoader().loadFromFile(
				core::assetPath(core::kScenesFile),
				core::kInitialRoom);
		world::Scenario scenario(config_, std::move(scene));
		const world::CharacterCatalog characters =
			world::CharacterCatalogLoader().loadFromFile(
				core::assetPath(core::kCharactersFile));
		const world::InventoryLayout inventory =
			world::InventoryConfigLoader().loadFromFile(
				core::assetPath(core::kInventoryFile));
		const world::ToolOverlayCatalog toolOverlay =
			world::ToolOverlayLoader().loadFromFile(
				core::assetPath(core::kToolsOverlayFile));
		const world::ResourceCatalog resources =
			world::ResourceConfigLoader().loadFromFile(
				core::assetPath(core::kResourcesFile));
		const world::GridSpan& grid = scenario.scene().grid;
		const int toolUseDuration =
			toolUseDurationTicks(characters);
		std::unique_ptr<game::GameClient> gameClient =
			createGameClient(grid, toolUseDuration);
		return runGameLoop(session, scenario, *gameClient, characters,
				   inventory, toolOverlay, resources);
	} catch (const world::ConfigError& error) {
		logger.error(error.what());
		return EXIT_FAILURE;
	} catch (const std::exception& error) {
		logger.error(error.what());
		return EXIT_FAILURE;
	}
}

std::unique_ptr<game::GameClient> Application::createGameClient(
	const world::GridSpan& grid, int toolUseDurationTicks) const {
	return std::make_unique<game::MockGameClient>(
		grid.columns, grid.rows, grid.columns / 2, grid.rows / 2,
		toolUseDurationTicks);
}

int Application::toolUseDurationTicks(
	const world::CharacterCatalog& characters) const {
	const world::CreatureData& creature =
		characters.dataFor(world::CreatureKind::Player);
	for (const world::DirectionAppearance& dir : creature.directions) {
		const auto it = dir.actions.find(world::ActionKind::UseTool);
		if (it != dir.actions.end()) {
			const int frames =
				static_cast<int>(it->second.frames.size());
			const int duration = frames * it->second.frameTicks;
			return duration > 0 ? duration : 1;
		}
	}
	return 1;
}

int Application::runGameLoop(const SessionConfig& session,
			     const world::Scenario& scenario,
			     game::GameClient& gameClient,
			     const world::CharacterCatalog& characters,
			     const world::InventoryLayout& inventory,
			     const world::ToolOverlayCatalog& toolOverlay,
			     const world::ResourceCatalog& resources) {

	(void)session;

	sdl::SdlContext sdlContext(config_);
	sdl::Renderer renderer(sdlContext, characters, toolOverlay, resources);
	sdl::InventoryHud inventoryHud(sdlContext, inventory);
	sdl::InputManager input(core::kInventorySlots);
	sdl::AudioPlayer audio;

	const StartupScreens screens(sdlContext, renderer, audio, scenario);
	const game::MoveCommand moveCommand;

	game::ViewMotion motion(characters);

	game::FootstepCue footstepCue;

	game::ResourceAudioCue resourceAudioCue;

	const world::CreatureData& localCreature =
		characters.dataFor(world::CreatureKind::Player);
	game::WalkPacer walkPacer(localCreature.cadence);

	constexpr std::size_t kPlayerIndex = 0;
	const game::ActionResolver actionResolver(motion);

	game::InventorySelector inventorySelector;

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
		}

		if (const std::optional<int> slot = input.takeSlotSelect()) {
			const int previous = inventorySelector.active();
			inventorySelector.select(*slot, core::kInventorySlots);
			if (inventorySelector.active() != previous) {
				audio.play(sdl::SoundId::ToolSwitch);
			}
		}

		const game::LocalWorldView& drawing = state;
		motion.observe(drawing, it);

		if (!drawing.entities.empty()) {
			const game::ViewEntity& player =
				drawing.entities[kPlayerIndex];
			const world::CreatureData& creature =
				characters.dataFor(player.kind);
			const world::AppearanceResolver resolver(creature);
			const int walkFrame = resolver.frameIndexFor(
				player.facing, world::ActionKind::Walk,
				motion.animationTickFor(kPlayerIndex, it));
			const bool walking = motion.isWalking(
				kPlayerIndex, it, player.moving);
			if (footstepCue.update(walking, walkFrame)) {
				audio.play(sdl::SoundId::Step);
			}
		}

		// Cambio visual del recurso (presentación). Diccionario, no
		// switch (por contrato).
		const std::map<game::ResourceAudioEvent, sdl::SoundId>
			resourceSounds = {
				{game::ResourceAudioEvent::Hit,
				 sdl::SoundId::MineralHit},
				{game::ResourceAudioEvent::Broke,
				 sdl::SoundId::MineralCrack},
			};
		const auto soundIt = resourceSounds.find(
			resourceAudioCue.update(drawing.resources));
		if (soundIt != resourceSounds.end()) {
			audio.play(soundIt->second);
		}

		// Obtención: evento que manda la fuente (no se deduce del
		// cambio visual).
		for (std::size_t i = 0; i < drawing.obtained.size(); ++i) {
			audio.play(sdl::SoundId::MineralObtained);
		}

		renderer.renderScene(scenario.scene(), drawing, motion,
				     actionResolver, it);
		inventoryHud.draw(drawing.inventory, inventorySelector.active());
		sdlContext.present();
	});

	loop.run();
	return 0;
}

}
