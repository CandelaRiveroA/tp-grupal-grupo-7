#include "core/PlaySession.h"

#include <cstddef>
#include <optional>

#include "core/Constants.h"
#include "core/StartupSequence.h"
#include "game/GameClient.h"
#include "game/GameConstants.h"
#include "game/LocalWorldView.h"
#include "sdl/loop/GameLoop.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"
#include "world/Scenario.h"
#include "world/ScenePixelMapper.h"
#include "world/ViewDirection.h"

namespace core {

PlaySession::PlaySession(const WindowConfig& config,
                         const world::Scenario& scenario,
                         game::GameClient& gameClient,
                         const world::CharacterCatalog& characters,
                         const world::InventoryLayout& inventory,
                         const world::ToolOverlayCatalog& toolOverlay,
                         const world::ResourceCatalog& resources,
                         const world::ObtainedBannerCatalog& banner,
                         const world::DigitCatalog& digits)
        : scenario_(scenario),
          gameClient_(gameClient),
          characters_(characters),
          sdlContext_(config),
          camera_(world::ScenePixelMapper(scenario.scene().cellPixelSize)
                          .worldBounds(scenario.scene().grid),
                  sdlContext_.width(), sdlContext_.height()),
          renderer_(sdlContext_, characters, toolOverlay, resources, inventory.content),
          inventoryHud_(sdlContext_, inventory, digits),
          bannerHud_(sdlContext_, banner, inventory.slotDrawWidth()),
          bannerQueue_(banner.durationTicks, banner.maxVisible),
          input_(core::kInventorySlots),
          screens_(sdlContext_, renderer_, audio_, scenario, camera_.viewport()),
          motion_(characters),
          actionResolver_(motion_),
          player_(gameClient,
                  characters.dataFor(world::CreatureKind::Player).cadence,
                  core::kInventorySlots),
          resourceSounds_{
                  {game::ResourceAudioEvent::Hit, sdl::SoundId::MineralHit},
                  {game::ResourceAudioEvent::Broke, sdl::SoundId::MineralCrack},
          } {
    renderer_.prepareScene(scenario.scene());
}

void PlaySession::run() {
    sdl::GameLoop loop(input_, [this](int iteration) { tick(iteration); });
    loop.run();
}

void PlaySession::tick(int iteration) {
    gameClient_.update(iteration);
    const game::LocalWorldView& state = gameClient_.worldState();
    motion_.observe(state, iteration);
    updateCamera(state, motion_, iteration);

    const GamePhase phase = phaseForIteration(iteration);
    if (screens_.draw(phase, iteration, state, motion_, actionResolver_)) {
        return;
    }

    const std::optional<world::Direction> pressed = input_.pressedDirection();

    // La intención del jugador: movimiento sostenido y one-shots (una
    // por pulsación). Usar la herramienta y recoger del piso son
    // INTENCIONES (cliente dummy): la fuente decide si hay algo y, si lo
    // hay, emite el evento obtained que dispara el panel + sonido.
    const game::PlayerEvents events =
            player_.tick(iteration, pressed, input_.takeCommands());
    if (events.toolSwitched) {
        audio_.play(sdl::SoundId::ToolSwitch);
    }

    if (!state.entities.empty()) {
        const game::ViewEntity& local = state.entities[game::kLocalPlayerIndex];
        const world::CreatureData& creature = characters_.dataFor(local.kind);
        const world::AppearanceResolver resolver(creature);
        const int walkFrame = resolver.frameIndexFor(
                local.facing, world::ActionKind::Walk,
                motion_.animationTickFor(game::kLocalPlayerIndex, iteration));
        const bool walking =
                motion_.isWalking(game::kLocalPlayerIndex, iteration, local.moving);
        if (footstepCue_.update(walking, walkFrame)) {
            audio_.play(sdl::SoundId::Step);
        }
    }

    // Un sonido por impacto confirmado, aunque el aspecto no cambie.
    for (const auto event : resourceAudioCue_.update(state.resourceHits)) {
        const auto soundIt = resourceSounds_.find(event);
        if (soundIt != resourceSounds_.end()) {
            audio_.play(soundIt->second);
        }
    }

    // Obtención: evento que manda la fuente (no se deduce del
    // cambio visual). Dispara el sonido y el cartel al mismo tiempo.
    for (std::size_t i = 0; i < state.obtained.size(); ++i) {
        audio_.play(sdl::SoundId::MineralObtained);
        bannerQueue_.push(state.obtained[i].kind);
    }
    bannerQueue_.tick();

    renderer_.drawWorld(scenario_.scene(), state, motion_, actionResolver_, iteration,
                        camera_.viewport());
    inventoryHud_.draw(state.inventory, player_.activeSlot());
    bannerHud_.draw(bannerQueue_.active());
    sdlContext_.present();
}

void PlaySession::updateCamera(const game::LocalWorldView& view,
                               const game::ViewMotion& motion,
                               int iteration) {
    if (view.entities.empty()) {
        return;
    }
    camera_.follow(motion.drawBoxFor(view.entities[game::kLocalPlayerIndex],
                                     game::kLocalPlayerIndex, iteration,
                                     scenario_.scene().cellPixelSize));
}

}  // namespace core
