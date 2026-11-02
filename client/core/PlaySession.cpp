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

PlaySession::PlaySession(const WindowConfig& config, const world::Scenario& scenario,
                         game::GameClient& gameClient, const world::CharacterCatalog& characters,
                         const world::InventoryLayout& inventory,
                         const world::ToolOverlayCatalog& toolOverlay,
                         const world::ResourceCatalog& resources,
                         const world::ObtainedBannerCatalog& banner,
                         const world::DigitCatalog& digits, const world::CursorCatalog& cursors,
                         const world::DialogCatalog& dialogs):
        scenario_(scenario),
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
        player_(gameClient, characters.dataFor(world::CreatureKind::Player).cadence,
                core::kInventorySlots),
        picker_(inventory.content),
        cursorCatalog_(cursors),
        cursorPainter_(sdlContext_),
        dialogCatalog_(dialogs),
        dialogPainter_(sdlContext_),
        resourceSounds_{
                {game::ResourceAudioEvent::Hit, sdl::SoundId::MineralHit},
                {game::ResourceAudioEvent::Broke, sdl::SoundId::MineralCrack},
        } {
    renderer_.prepareScene(scenario.scene());
}

void PlaySession::run() {
    sdl::GameLoop loop(input_, [this, &loop](int iteration) {
        tick(iteration);
        if (!gameClient_.connected()) {
            loop.stop();
        }
    });
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
    const game::PlayerEvents events = player_.tick(iteration, pressed, input_.takeCommands());
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
        const bool walking = motion_.isWalking(game::kLocalPlayerIndex, iteration, local.moving);
        if (footstepCue_.update(walking, walkFrame)) {
            audio_.play(sdl::SoundId::Step);
        }
    }

    // Un sonido por impacto confirmado, aunque el aspecto no cambie.
    for (const auto event: resourceAudioCue_.update(state.resourceHits)) {
        const auto soundIt = resourceSounds_.find(event);
        if (soundIt != resourceSounds_.end()) {
            audio_.play(soundIt->second);
        }
    }

    // Obtención: evento que manda la fuente (no se deduce del
    // cambio visual). Dispara el sonido y el cartel al mismo tiempo.
    for (std::size_t i = 0; i < state.obtained.size(); ++i) {
        audio_.play(sdl::SoundId::MineralObtained);
        bannerQueue_.push(state.obtained[i].contentId);
    }

    // Sonidos del mundo (los cues de los animales): los emite la fuente
    // en el estado, el cliente solo los reproduce. El filtro por sala
    // (game::SoundGate) se conecta cuando haya más de una (el granero).
    for (const game::ViewSoundEvent& event: state.soundEvents) {
        audio_.playCue(event.cue);
    }
    bannerQueue_.tick();

    renderer_.drawWorld(scenario_.scene(), state, motion_, actionResolver_, iteration,
                        camera_.viewport());

    // Diálogos de las entidades (lo que "dicen"): la indicación viene en
    // el estado y el catálogo dice cómo se ve. Sobre el mundo, después
    // de las entidades.
    dialogAnimator_.observe(state, iteration);
    for (std::size_t i = 0; i < state.entities.size(); ++i) {
        const game::ViewEntity& entity = state.entities[i];
        if (!entity.indication.has_value()) {
            continue;
        }
        const world::PixelRect entityBox =
                motion_.drawBoxFor(entity, i, iteration, scenario_.scene().cellPixelSize);
        dialogPainter_.draw(dialogCatalog_, *entity.indication, entityBox,
                            dialogAnimator_.animationTickFor(entity, iteration),
                            camera_.viewport());
    }

    inventoryHud_.draw(state.inventory, player_.activeSlot());
    bannerHud_.draw(bannerQueue_.active());

    // El puntero: la variante la decide el estado de juego (si hay algo
    // interactivo bajo el cursor); el painter solo dibuja. La posición
    // del click se traduce a mundo con el Viewport, dueño de esa
    // conversión.
    const world::PixelPoint worldCursor = camera_.viewport().toWorld(
            world::PixelPoint{input_.mousePosition().x, input_.mousePosition().y});
    const game::PickTarget target =
            picker_.pick(worldCursor, state, motion_, iteration, scenario_.scene().cellPixelSize);

    // El click sobre algo del mundo manda la intención de tomarlo: la
    // fuente decide qué pasa (el ordeñe, por ejemplo, valida cercanía).
    for (const game::PointerClick& click: input_.takeClicks()) {
        const world::PixelPoint worldClick =
                camera_.viewport().toWorld(world::PixelPoint{click.x, click.y});
        const game::PickTarget clicked = picker_.pick(worldClick, state, motion_, iteration,
                                                      scenario_.scene().cellPixelSize);
        if (clicked.kind != game::PickTarget::Kind::None) {
            player_.interact();
        }
    }

    cursorPainter_.draw(cursorCatalog_,
                        isInteractive(target, state) ? world::CursorKind::Interactive :
                                                       world::CursorKind::Normal,
                        input_.mousePosition());
    sdlContext_.present();
}

void PlaySession::updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                               int iteration) {
    if (view.entities.empty()) {
        return;
    }
    camera_.follow(motion.drawBoxFor(view.entities[game::kLocalPlayerIndex],
                                     game::kLocalPlayerIndex, iteration,
                                     scenario_.scene().cellPixelSize));
}

bool PlaySession::isInteractive(const game::PickTarget& target,
                                const game::LocalWorldView& view) const {
    // El estado dice qué es interactivo; esto solo lo lee.
    if (target.kind == game::PickTarget::Kind::Entity) {
        return view.entities[target.index].interactive;
    }
    if (target.kind == game::PickTarget::Kind::DroppedItem) {
        return view.droppedItems[target.index].interactive;
    }
    return false;
}

}  // namespace core
