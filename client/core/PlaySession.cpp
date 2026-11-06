#include "core/PlaySession.h"

#include <cstddef>
#include <optional>
#include <vector>

#include "core/Constants.h"
#include "core/FarmConstants.h"
#include "core/StartupSequence.h"
#include "game/GameClient.h"
#include "game/GameConstants.h"
#include "game/LocalWorldView.h"
#include "sdl/loop/GameLoop.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"
#include "world/Scenario.h"
#include "world/SceneConfigLoader.h"
#include "world/ScenePixelMapper.h"
#include "world/ViewDirection.h"
#include "world/WorldConstants.h"

namespace core {

PlaySession::PlaySession(const WindowConfig& config, const world::Scenario& scenario,
                         game::GameClient& gameClient, const world::CharacterCatalog& characters,
                         const world::InventoryLayout& inventory,
                         const world::ToolOverlayCatalog& toolOverlay,
                         const world::ResourceCatalog& resources,
                         const world::ObtainedBannerCatalog& banner,
                         const world::DigitCatalog& digits, const world::CursorCatalog& cursors,
                         const world::DialogCatalog& dialogs, const world::FontCatalog& font,
                         const world::PromptLayout& prompt,
                         const world::HudOverlayCatalog& hudOverlays,
                         const world::HudOverlayCatalog& lighting):
        scene_(scenario.scene()),
        gameClient_(gameClient),
        characters_(characters),
        farm_(core::assetPath(core::kCropsFile), core::assetPath(core::kGroundFile),
              core::assetPath(core::kBuildPlaceFile)),
        inventory_(farm_.withCropContents(inventory)),
        sdlContext_(config),
        camera_(world::ScenePixelMapper(scenario.scene().cellPixelSize)
                        .worldBounds(scenario.scene().grid),
                sdlContext_.width(), sdlContext_.height()),
        renderer_(sdlContext_, characters, toolOverlay, resources, inventory_.content,
                  farm_.ground(), farm_.crops()),
        inventoryHud_(sdlContext_, inventory_, digits),
        bannerHud_(sdlContext_, banner, inventory_.slotDrawWidth()),
        textRenderer_(sdlContext_, font),
        hudOverlayPainter_(sdlContext_, textRenderer_, hudOverlays),
        lightingPainter_(sdlContext_, textRenderer_, lighting),
        bannerQueue_(banner.durationTicks, banner.maxVisible),
        input_(inventory_.count),
        screens_(sdlContext_, renderer_, audio_, scenario, camera_.viewport()),
        motion_(characters),
        farmSelection_(inventory_.traits),
        enterSelection_(scenario.scene(), scenario.scene().cellPixelSize),
        popupState_(),
        sceneTransition_(core::kFadeTicks),
        dayPhase_(core::kLightTransitionTicks),
        testStages_{
                {game::InputCommand::Kind::TestDusk, world::TimeOfDay::Dusk},
                {game::InputCommand::Kind::TestEvening, world::TimeOfDay::Evening},
                {game::InputCommand::Kind::TestNight, world::TimeOfDay::Night},
        },
        actionResolver_(motion_, farmSelection_.carry()),
        player_(gameClient, characters.dataFor(world::CreatureKind::Player).cadence,
                inventory_.count),
        picker_(inventory_.content),
        markerPainter_(sdlContext_),
        cursorCatalog_(cursors),
        cursorPainter_(sdlContext_),
        dialogCatalog_(dialogs),
        dialogPainter_(sdlContext_),
        promptLayout_(prompt),
        promptPainter_(sdlContext_, textRenderer_, promptLayout_),
        // PLACEHOLDER: valores crudos que hoy se inventan para ver las
        // etiquetas de hud.toml. Mañana los arma la estructura que guarda
        // lo que manda el servidor; cada clave es un {token} (o `source`)
        // de hud.toml. Es el ÚNICO punto a cambiar.
        hudValues_{
                {"dia_semana", 0},
                {"dia", 1},
                {"hora", 6},
                {"minuto", 30},
        },
        resourceSounds_{
                {game::ResourceAudioEvent::Hit, sdl::SoundId::MineralHit},
                {game::ResourceAudioEvent::Broke, sdl::SoundId::MineralCrack},
        } {
    renderer_.prepareScene(scene_);
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
    dayPhase_.advance();

    const GamePhase phase = phaseForIteration(iteration);
    if (screens_.draw(phase, iteration, state, motion_, actionResolver_,
                      dayPhase_.phase())) {
        return;
    }

    // La transición de escena manda: el "apagado y vuelta a prender".
    // En el negro cambian los datos; mientras está activa se dibuja el
    // mundo con el fade encima y NO hay input (las colas se drenan para
    // que nada se dispare al volver).
    sceneTransition_.tick(iteration);
    if (sceneTransition_.swapDue()) {
        changeScene(sceneTransition_.scene());
        sceneTransition_.consumeSwap();
    }
    if (sceneTransition_.active()) {
        input_.takeCommands();
        input_.takeClicks();
        renderer_.drawWorld(scene_, enterSelection_.displayView(state), motion_,
                            actionResolver_, iteration, camera_.viewport(),
                            dayPhase_.phase());
        lightingPainter_.draw(dayPhase_.nightFactor());
        sdlContext_.fillRect(
                SDL2pp::Rect(0, 0, sdlContext_.width(), sdlContext_.height()),
                SDL2pp::Color(0, 0, 0, static_cast<Uint8>(sceneTransition_.alpha())));
        sdlContext_.present();
        return;
    }

    const std::optional<world::Direction> pressed = input_.pressedDirection();

    // La intención del jugador: movimiento sostenido y one-shots (una
    // por pulsación). Usar la herramienta y recoger del piso son
    // INTENCIONES (cliente dummy): la fuente decide si hay algo y, si lo
    // hay, emite el evento obtained que dispara el panel + sonido.
    const std::vector<game::InputCommand> commands = input_.takeCommands();
    // El atajo de prueba (borrar cuando el server mande esto): la tecla
    // Y simula la información de avance de "dormir" que mañana enviará
    // la fuente de estado por PopupState::showWaiting. El resto de los
    // comandos son intenciones del jugador.
    std::vector<game::InputCommand> playerCommands;
    for (const game::InputCommand& command: commands) {
        if (command.kind == game::InputCommand::Kind::TestWaiting) {
            popupState_.showWaiting(1, 3);  // dummy: 1 de 3 dormidos
            continue;
        }
        if (command.kind == game::InputCommand::Kind::TestSceneChange) {
            // Atajo de prueba (borrar cuando el server lo llame):
            // simula "el server dice: nuevo día" — la transición al
            // dormitorio y el gallo que la fuente emitirá por sus
            // soundEvents. No arma nada: solo se vuelve a pintar la
            // escena con lo que la vista diga.
            sceneTransition_.changeTo(core::kInitialRoom);
            audio_.play(sdl::SoundId::RoosterCrow);
            // El nuevo día vuelve a la luz del día (ver DayPhase).
            dayPhase_.setPhase(world::TimeOfDay::Day);
            continue;
        }
        if (const auto stage = testStages_.find(command.kind);
            stage != testStages_.end()) {
            // Atajo de prueba (borrar cuando el server lo llame): fija
            // el momento del día. Mañana lo setea la conexión con lo
            // que el server mande, llamando al mismo
            // DayPhase::setPhase.
            dayPhase_.setPhase(stage->second);
            continue;
        }
        if (popupState_.modal()) {
            // El popup manda: el resto de las intenciones se descartan
            // (no se encolan para cuando cierre).
            continue;
        }
        playerCommands.push_back(command);
    }
    // Mientras el popup es modal no hay input manual: la auto-caminata
    // en curso (el "meterse") termina su paso y después el personaje
    // queda quieto hasta que se responda.
    const game::PlayerEvents events = player_.tick(
            iteration, popupState_.modal() ? std::nullopt : pressed,
            playerCommands);
    if (events.toolSwitched) {
        audio_.play(sdl::SoundId::ToolSwitch);
    }

    // Presentación local del campo (saco en la cabeza y lugar de
    // plantado): sobre la selección fresca de este tick, y la celda que
    // apunta el puntero (el Viewport traduce a mundo).
    const world::ScenePixelMapper cells(scene_.cellPixelSize);
    const world::PixelPoint worldCursor = camera_.viewport().toWorld(
            world::PixelPoint{input_.mousePosition().x, input_.mousePosition().y});
    // La caja del popup (figura del inventario centrada, en la grilla
    // que digan sus datos): donde se dibuja y donde se tocan sus cosas.
    const world::PixelBox popupBox = inventory_.centeredBox(
            sdlContext_.width(), sdlContext_.height(), promptLayout_.grid.columns,
            promptLayout_.grid.rows);
    farmSelection_.observe(state, player_.activeSlot(), cells.toCell(worldCursor));

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
        if (const std::optional<sdl::SoundId> sound = toolUseSounds_.update(local.activeTool)) {
            audio_.play(*sound);
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

    // El mundo se dibuja con la vista "puesta": si el jugador está
    // metido en un elemento, en su pose (presentación local; ver
    // EnterSelection). Encima, la capa frontal que lo tapa.
    renderer_.drawWorld(scene_, enterSelection_.displayView(state),
                        motion_, actionResolver_, iteration,
                        camera_.viewport(), dayPhase_.phase());
    drawEnteredFront(state, cells);
    drawPlantingMarker(cells);

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
                motion_.drawBoxFor(entity, i, iteration, scene_.cellPixelSize);
        dialogPainter_.draw(dialogCatalog_, *entity.indication, entityBox,
                            dialogAnimator_.animationTickFor(entity, iteration),
                            camera_.viewport());
    }
    // Y los de los cultivos (p. ej. piden agua): mismo mecanismo, el
    // dueño es la celda de tierra donde se dibuja la planta.
    for (const game::ViewCrop& crop: state.plantedCrops) {
        if (!crop.indication.has_value()) {
            continue;
        }
        const world::PixelRect cropBox = cells.toPixels(
                world::GridCell{crop.cellX, crop.cellY}, world::kUnitCellSpan);
        dialogPainter_.draw(dialogCatalog_, *crop.indication, cropBox,
                            dialogAnimator_.animationTickFor(crop, iteration),
                            camera_.viewport());
    }

    // Luz nocturna: es el corte mundo/HUD. Todo lo anterior es mundo
    // (queda tintado); todo lo posterior es interfaz fija (no).
    lightingPainter_.draw(dayPhase_.nightFactor());
    inventoryHud_.draw(state.inventory, player_.activeSlot());
    bannerHud_.draw(bannerQueue_.active());
    // Overlays de HUD fijos (arriba a la derecha, etc.): sin cámara,
    // antes del popup y del puntero para que queden encima.
    hudOverlayPainter_.draw();
    hudOverlayPainter_.drawLabels(hudValues_);
    drawPopup(popupBox);

    // El puntero: la variante la decide el estado de juego (si hay algo
    // interactivo bajo el cursor); el painter solo dibuja.
    const game::PickTarget target =
            picker_.pick(worldCursor, state, motion_, iteration, scene_.cellPixelSize);

    // El click sobre algo interactivo manda la intención de tomarlo; el
    // resto (piso, rocas, nada) cae al uso de herramienta hacia esa
    // celda. La fuente decide qué pasa en ambos casos. Antes que todo:
    // el click sobre un elemento entrable lo mete/saca (presentación
    // local) y se consume ahí.
    for (const game::PointerClick& click: input_.takeClicks()) {
        const world::PixelPoint worldClick =
                camera_.viewport().toWorld(world::PixelPoint{click.x, click.y});
        // El botón de cancelar del popup "Esperando..." (solo ese
        // popup): cancelar es una intención hacia la fuente de estado
        // (stand-in local, igual que el voto — mañana la manda la
        // conexión). A nivel dibujo: el popup se va y el jugador sigue
        // su vida normal (sale de la cama caminando cuando quiere).
        if (popupState_.view() == game::PopupView::Waiting &&
                promptLayout_.cancelAt(popupBox, world::PixelPoint{click.x, click.y})) {
            popupState_.hide();
            continue;
        }
        // El popup con la pregunta está encima de todo: sus opciones se
        // resuelven primero (coords de pantalla: es HUD), con las
        // MISMAS cajas que se dibujan. Cualquier opción es un voto:
        // cierra la pregunta; la consecuencia la decidirá la fuente de
        // estado (fuera de alcance).
        if (popupState_.view() == game::PopupView::Question &&
                promptLayout_.optionAt(popupBox, world::PixelPoint{click.x, click.y}) >= 0) {
            popupState_.hide();
            continue;
        }
        // Modal: con el popup abierto el mundo está bloqueado; el resto
        // de los clicks no hace nada (se drenan para que no se disparen
        // en cascada al cerrar).
        if (popupState_.modal()) {
            continue;
        }
        // La cama. El disparador del popup es LOCAL (stand-in temporal,
        // igual que la tecla Y): mañana es el server el que decide qué
        // mostrar y llama PopupState::showQuestion()/showWaiting(). La
        // pregunta solo aparece si el jugador está en la zona de la
        // cama SEGÚN LA VISTA (la posición la manda la fuente de
        // estado); tocarla no saca a nadie.
        const game::EnterClick enterClick = enterSelection_.clickAt(worldClick, state);
        if (enterClick.hit) {
            if (enterClick.ask) {
                popupState_.showQuestion();
            }
            if (enterClick.walkTo.has_value()) {
                player_.walkTo(*enterClick.walkTo);
            }
            continue;
        }
        const game::PickTarget clicked = picker_.pick(worldClick, state, motion_, iteration,
                                                      scene_.cellPixelSize);
        if (clicked.kind != game::PickTarget::Kind::None && isInteractive(clicked, state)) {
            player_.interact();
            continue;
        }
        player_.useToward(cells.toCell(worldClick), pressed);
    }

    // El puntero: mientras el popup es modal, el mundo está bloqueado y
    // solo es apretable lo del popup (las opciones de la pregunta o el
    // botón de cancelar).
    const world::PixelPoint mousePoint{input_.mousePosition().x, input_.mousePosition().y};
    bool interactive = isInteractive(target, state) ||
                       enterSelection_.hits(worldCursor, state);
    if (popupState_.modal()) {
        interactive = (popupState_.view() == game::PopupView::Question &&
                       promptLayout_.optionAt(popupBox, mousePoint) >= 0) ||
                      (popupState_.view() == game::PopupView::Waiting &&
                       promptLayout_.cancelAt(popupBox, mousePoint));
    }
    cursorPainter_.draw(cursorCatalog_,
                        interactive ? world::CursorKind::Interactive :
                                      world::CursorKind::Normal,
                        input_.mousePosition());
    sdlContext_.present();
}

void PlaySession::drawPlantingMarker(const world::ScenePixelMapper& cells) {
    // QUÉ celda y en qué estado la marca lo resolvió FarmSelection; esto
    // solo la pinta, sobre el mundo y como una selección más del HUD.
    if (const std::optional<game::PlantingSpot>& spot = farmSelection_.spot()) {
        markerPainter_.draw(farm_.buildPlaces(), spot->state, spot->cell, cells,
                            camera_.viewport());
    }
}

void PlaySession::drawEnteredFront(const game::LocalWorldView& view,
                                   const world::ScenePixelMapper& cells) {
    // Las filas frontales del elemento donde está metido el jugador (lo
    // que lo tapa): QUÉ dibujar lo resolvió EnterSelection; esto solo
    // lo pinta, sobre las entidades y bajo el HUD.
    const std::optional<game::EnteredFront> front = enterSelection_.front(view);
    if (!front.has_value()) {
        return;
    }
    renderer_.drawElementFront(scene_,
                               scene_.elements[front->elementIndex],
                               cells, camera_.viewport(),
                               front->firstRow, front->lastRow);
}

void PlaySession::changeScene(const std::string& room) {
    // El cambio en el negro de la transición: cambia QUÉ escena se
    // pinta (el dibujo es el mismo). La posición del personaje y todo
    // lo demás lo sigue mandando la fuente de estado por la vista.
    scene_ = world::SceneConfigLoader().loadFromFile(
            core::assetPath(core::kScenesFile), room);
    renderer_.prepareScene(scene_);
    enterSelection_.reindex(scene_, scene_.cellPixelSize);
    camera_.setWorldBounds(world::ScenePixelMapper(scene_.cellPixelSize)
                                   .worldBounds(scene_.grid));
}

void PlaySession::drawPopup(const world::PixelBox& panel) {
    // El popup: la figura del inventario (marco + cuadros) y, encima, el
    // contenido que diga PopupState (la pregunta con sus opciones o el
    // avance). La opción hovereada se resuelve con las MISMAS cajas que
    // se dibujan (PromptLayout); el cuándo lo decide PopupState, el
    // cómo lo pintan los painters.
    if (popupState_.view() == game::PopupView::Hidden) {
        return;
    }
    inventoryHud_.drawPanel(panel, promptLayout_.grid.columns,
                            promptLayout_.grid.rows);
    const world::PixelPoint mouse{input_.mousePosition().x, input_.mousePosition().y};
    promptPainter_.draw(popupState_, promptLayout_.optionAt(panel, mouse), panel);
}

void PlaySession::updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                               int iteration) {
    if (view.entities.empty()) {
        return;
    }
    camera_.follow(motion.drawBoxFor(view.entities[game::kLocalPlayerIndex],
                                     game::kLocalPlayerIndex, iteration,
                                     scene_.cellPixelSize));
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
    if (target.kind == game::PickTarget::Kind::Crop) {
        return view.plantedCrops[target.index].interactive;
    }
    return false;
}

}  // namespace core
