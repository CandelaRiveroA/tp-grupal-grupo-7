#include "core/PlaySession.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utf8.h>
#include <vector>

#include "core/Constants.h"
#include "core/FarmConstants.h"
#include "core/StartupSequence.h"
#include "game/GameClient.h"
#include "game/GameConstants.h"
#include "game/EnemyTestTrigger.h"
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
                         const world::PromptLayout& prompt, const world::ChatLayout& chat,
                         const world::HudOverlayCatalog& hudOverlays,
                         const world::HudOverlayCatalog& lighting,
                         const world::CharacterPalettes& palettes,
                         game::EnemyTestTrigger* enemyTestTrigger):
        scene_(scenario.scene()),
        gameClient_(gameClient),
        enemyTestTrigger_(enemyTestTrigger),
        testActions_{{game::InputCommand::Kind::TestMove, world::ActionKind::Walk},
                     {game::InputCommand::Kind::TestAttack, world::ActionKind::Attack},
                     {game::InputCommand::Kind::TestDamage, world::ActionKind::Damage},
                     {game::InputCommand::Kind::TestDeath, world::ActionKind::Death},
                     {game::InputCommand::Kind::TestRevive,
                      world::ActionKind::Revive}},
        projectileTests_{
                {game::InputCommand::Kind::TestProjectileAdvance,
                 &game::EnemyTestTrigger::advanceProjectiles},
                {game::InputCommand::Kind::TestProjectileDespawn,
                 &game::EnemyTestTrigger::despawnProjectiles}},
        attackProjectiles_{{game::InputCommand::Kind::TestGreenFire,
                            world::CreatureKind::SkullFireGreen}},
        characters_(characters),
        farm_(core::assetPath(core::kCropsFile), core::assetPath(core::kGroundFile),
              core::assetPath(core::kBuildPlaceFile)),
        inventory_(farm_.withCropContents(inventory)),
        sdlContext_(config),
        camera_(world::ScenePixelMapper(scenario.scene().cellPixelSize)
                        .worldBounds(scenario.scene().grid),
                sdlContext_.width(), sdlContext_.height()),
        renderer_(sdlContext_, characters, toolOverlay, resources, inventory_.content,
                  farm_.ground(), farm_.crops(), palettes),
        inventoryHud_(sdlContext_, inventory_, digits),
        bannerHud_(sdlContext_, banner, inventory_.slotDrawWidth()),
        textRenderer_(sdlContext_, font),
        hudOverlayPainter_(sdlContext_, textRenderer_, hudOverlays),
        lightingPainter_(sdlContext_, textRenderer_, lighting),
        bannerQueue_(banner.durationTicks, banner.maxVisible),
        input_(inventory_.count),
        screens_(sdlContext_, renderer_, audio_, scenario, camera_.viewport()),
        motion_(characters),
        actionSounds_(characters),
        farmSelection_(inventory_.traits),
        enterSelection_(scenario.scene(), scenario.scene().cellPixelSize),
        popupState_(),
        sceneTransition_(core::kFadeTicks),
        dayPhase_(core::kLightTransitionTicks),
        stats_{
                {game::StatKind::Energy, game::FractionStat{}},
                {game::StatKind::Life, game::FractionStat{}},
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
        chatPainter_(sdlContext_, textRenderer_, chat),
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
    // La presentación local del tick (la pose de "meterse") es la que
    // ven el movimiento, el renderer y los diálogos.
    const game::LocalWorldView presentation = enterSelection_.displayView(state);
    motion_.observe(presentation, iteration);
    updateCamera(presentation, motion_, iteration);
    // El reloj del server decide la etapa del día y lo que muestra el HUD.
    dayPhase_.setClock(state.dayNumber, state.clockMinutes);
    dayPhase_.advance();

    // Se terminó el día: se cierra todo y la pantalla se funde a negro y SE
    // QUEDA ahí con el popup del nuevo día hasta que despierten todos. Ahí
    // (ServerDay) cambia la escena (dormitorio), vuelve el fade y suena el gallo.
    // Va ANTES de la secuencia de inicio: un jugador que se une durante el
    // popup recibe el evento mientras todavía está en la intro.
    if (state.dayEnded) {
        voted_ = false;
        woke_ = false;
        popupState_.hide();
        sceneTransition_.changeTo(core::kInitialRoom, true);
    }
    if (state.dayChanged) {
        sceneTransition_.release();
        audio_.play(sdl::SoundId::RoosterCrow);
        // Día nuevo: los stats simulados arrancan llenos (lo dirá el server).
        for (auto& entry : stats_) {
            entry.second.reset();
        }
    }

    const GamePhase phase = phaseForIteration(iteration);
    if (screens_.draw(phase, iteration, state, motion_, actionResolver_, dayPhase_.phase())) {
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
    // La caja del popup (figura del inventario centrada, en la grilla
    // que digan sus datos): donde se dibuja y donde se tocan sus cosas.
    const world::PixelBox popupBox =
            inventory_.centeredBox(sdlContext_.width(), sdlContext_.height(),
                                   promptLayout_.grid.columns, promptLayout_.grid.rows);
    if (sceneTransition_.active()) {
        input_.takeCommands();
        // El amanecer manda: con el chat abierto el OK queda intocable
        // (los comandos se drenan y el modo texto come los clicks).
        if (input_.chatOpen()) {
            closeChat();
        }
        const std::vector<game::PointerClick> clicks = input_.takeClicks();
        const bool holding = sceneTransition_.holding();
        if (holding) {
            // Sostenida en negro: el popup del nuevo día es lo único que
            // se puede tocar (como el de la cama, es modal). Sin cancelar.
            if (!woke_) {
                popupState_.showNextDay();
            }
            for (const game::PointerClick& click: clicks) {
                if (woke_ || !promptLayout_.buttonAt(world::kPromptOkButton, popupBox,
                                                     world::PixelPoint{click.x, click.y})) {
                    continue;
                }
                // OK: el server espera a que despierten todos; el negro se
                // suelta cuando llega ServerDay.
                woke_ = true;
                popupState_.hide();
                gameClient_.sendCommand(protocol::WakeMessage{});
                break;
            }
        } else {
            renderer_.drawWorld(scene_, presentation, motion_, actionResolver_, iteration,
                                camera_.viewport(), dayPhase_.phase());
            lightingPainter_.draw({}, dayPhase_.nightFactor());
        }
        sdlContext_.fillRect(SDL2pp::Rect(0, 0, sdlContext_.width(), sdlContext_.height()),
                             SDL2pp::Color(0, 0, 0, static_cast<Uint8>(sceneTransition_.alpha())));
        if (holding) {
            drawPopup(popupBox);
            const world::PixelPoint mouse{input_.mousePosition().x, input_.mousePosition().y};
            cursorPainter_.draw(cursorCatalog_,
                                promptLayout_.buttonAt(world::kPromptOkButton, popupBox, mouse) ?
                                        world::CursorKind::Interactive :
                                        world::CursorKind::Normal,
                                input_.mousePosition());
        }
        sdlContext_.present();
        return;
    }

    const std::optional<world::Direction> pressed = input_.pressedDirection();

    // La intención del jugador: movimiento sostenido y one-shots (una
    // por pulsación). Usar la herramienta y recoger del piso son
    // INTENCIONES (cliente dummy): la fuente decide si hay algo y, si lo
    // hay, emite el evento obtained que dispara el panel + sonido.
    // Sueño: el server manda el aviso (ServerSleep) solo al cambiar, así
    // que se reacciona a la transición y no a cada tick.
    // Muerto (vida o energía): el server lo da por aceptado en la cama y
    // no puede levantarse, así que espera sin pregunta y sin cancelar.
    const bool localDied =
            (state.playerMaxHealth > 0 && state.playerHealth == 0) ||
            state.playerFainted;
    popupState_.setWaitingCancelable(!localDied);
    if (state.sleep.sleeping != lastSleeping_ || state.sleep.total != lastTotal_) {
        lastSleeping_ = state.sleep.sleeping;
        lastTotal_ = state.sleep.total;
        if (lastSleeping_ == 0) {
            voted_ = false;
        }
        popupState_.showSleepState(lastSleeping_, lastTotal_, voted_, localDied);
    }
    // Murió con la pregunta abierta (sin flanco de sueño: el sleepers no
    // cambió): la pregunta salta al avance, que es lo que el server ya
    // registró.
    if (localDied && popupState_.view() == game::PopupView::Question) {
        voted_ = true;
        popupState_.showWaiting(lastSleeping_, lastTotal_);
    }

    // Lo tipeado y los backspaces del frame entran al buffer.
    if (input_.chatOpen()) {
        chatInput_ += input_.takeText();
        // Tope del texto tipeado: un paste largo encarece el wrap.
        while (chatInput_.size() > core::kChatMaxLength) {
            eraseLastCodepoint(chatInput_);
        }
        for (int i = input_.takeBackspaces(); i > 0; --i) {
            eraseLastCodepoint(chatInput_);
        }
    }

    // Llegó un mensaje: el preview arranca su cuenta desde acá.
    if (state.chatVersion != seenChatVersion_) {
        seenChatVersion_ = state.chatVersion;
        chatPreviewUntilTick_ = iteration + core::kChatPreviewTicks;
    }

    const std::vector<game::InputCommand> commands = input_.takeCommands();
    std::vector<game::InputCommand> playerCommands;
    // ANDAMIAJE TEMPORAL (prueba): las teclas G/F/H/K disparan acciones
    // de los enemigos en el mundo simulado (si hay), J/L mueven/borran
    // sus proyectiles y V hace que el Skull ataque con el fuego verde.
    // El server real pondrá la acción/posición directo en la vista y
    // esto se saca.
    for (const game::InputCommand& command: commands) {
        // Enter manda y deja el chat ABIERTO; ESC lo cierra. Los dos
        // pasan aunque el popup sea modal.
        if (command.kind == game::InputCommand::Kind::ChatSubmit) {
            if (!chatInput_.empty()) {
                gameClient_.sendChat(chatInput_);
                chatInput_.clear();
            }
            continue;
        }
        if (command.kind == game::InputCommand::Kind::ChatCancel) {
            closeChat();
            continue;
        }
        const auto test = testActions_.find(command.kind);
        if (test != testActions_.end()) {
            if (enemyTestTrigger_ != nullptr) {
                enemyTestTrigger_->triggerAll(test->second);
            }
            continue;
        }
        const auto projectileTest = projectileTests_.find(command.kind);
        if (projectileTest != projectileTests_.end()) {
            if (enemyTestTrigger_ != nullptr) {
                (enemyTestTrigger_->*(projectileTest->second))();
            }
            continue;
        }
        const auto attackProjectile = attackProjectiles_.find(command.kind);
        if (attackProjectile != attackProjectiles_.end()) {
            if (enemyTestTrigger_ != nullptr) {
                enemyTestTrigger_->triggerAttackWith(attackProjectile->second);
            }
            continue;
        }
        if (command.kind == game::InputCommand::Kind::OpenChat) {
            input_.setTextMode(true);
            sdlContext_.startTextInput();
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
    const game::PlayerEvents events =
            player_.tick(iteration, popupState_.modal() ? std::nullopt : pressed, playerCommands);
    if (events.toolSwitched) {
        audio_.play(sdl::SoundId::ToolSwitch);
    }

    // Presentación local del campo (saco en la cabeza y lugar de
    // plantado): sobre la selección fresca de este tick, y la celda que
    // apunta el puntero (el Viewport traduce a mundo).
    const world::ScenePixelMapper cells(scene_.cellPixelSize);
    const world::PixelPoint worldCursor = camera_.viewport().toWorld(
            world::PixelPoint{input_.mousePosition().x, input_.mousePosition().y});
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

    // Un sonido por cada acción que ACABA de empezar (el daño recibido:
    // golpe al jugador o al enemigo, según la criatura), o al entrar a
    // su último cuadro (el momento lo decide la tabla de ActionSounds).
    // Va sobre la vista de PRESENTACIÓN: así suenan también las
    // acciones locales.
    for (const sdl::SoundId sound : actionSounds_.update(presentation, iteration)) {
        audio_.play(sound);
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
    renderer_.drawWorld(scene_, presentation, motion_, actionResolver_, iteration,
                        camera_.viewport(), dayPhase_.phase());
    drawEnteredFront(presentation, cells);
    drawPlantingMarker(cells);

    // Diálogos de las entidades (lo que "dicen"): la indicación viene en
    // el estado y el catálogo dice cómo se ve. Sobre el mundo, después
    // de las entidades.
    dialogAnimator_.observe(presentation, iteration);
    for (std::size_t i = 0; i < presentation.entities.size(); ++i) {
        const game::ViewEntity& entity = presentation.entities[i];
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
    for (const game::ViewCrop& crop: presentation.plantedCrops) {
        if (!crop.indication.has_value()) {
            continue;
        }
        const world::PixelRect cropBox =
                cells.toPixels(world::GridCell{crop.cellX, crop.cellY}, world::kUnitCellSpan);
        dialogPainter_.draw(dialogCatalog_, *crop.indication, cropBox,
                            dialogAnimator_.animationTickFor(crop, iteration), camera_.viewport());
    }

    // Luz nocturna: es el corte mundo/HUD. Todo lo anterior es mundo
    // (queda tintado); todo lo posterior es interfaz fija (no).
    lightingPainter_.draw({}, dayPhase_.nightFactor());
    inventoryHud_.draw(state.inventory, player_.activeSlot());
    bannerHud_.draw(bannerQueue_.active());
    // Overlays de HUD fijos (arriba a la derecha, etc.): sin cámara,
    // antes del popup y del puntero para que queden encima.
    std::map<std::string, double> hudFillValues;
    // Con el server en línea manda su vida/energía; sin server (offline)
    // sigue mandando la simulación local.
    if (state.playerMaxHealth > 0) {
        hudFillValues[game::kHudLifeKey] =
                static_cast<double>(state.playerHealth) / state.playerMaxHealth;
        if (state.playerMaxEnergy > 0) {
            hudFillValues[game::kHudEnergyKey] =
                    static_cast<double>(state.playerEnergy) / state.playerMaxEnergy;
        }
    } else {
        hudFillValues[game::kHudEnergyKey] =
                stats_.at(game::StatKind::Energy).fraction();
        hudFillValues[game::kHudLifeKey] =
                stats_.at(game::StatKind::Life).fraction();
    }
    hudOverlayPainter_.draw(hudFillValues);
    hudOverlayPainter_.drawLabels(dayPhase_.clockValues());
    // Abierto: historial + tipeo (el cursor parpadea con el tick).
    // Cerrado: el preview atenuado, que se apaga solo.
    if (input_.chatOpen()) {
        chatPainter_.drawOpen(chatInput_, state.chatLog,
                              (iteration / core::kChatBlinkTicks) % 2 == 0);
    } else if (iteration < chatPreviewUntilTick_) {
        chatPainter_.drawPreview(state.chatLog);
    }
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
        // El botón de cancelar del popup "Esperando..." sale de la cama
        // (el server borra el OK de este jugador). Un muerto no cancela.
        if (popupState_.view() == game::PopupView::Waiting &&
            popupState_.waitingCancelable() &&
            promptLayout_.buttonAt(world::kPromptCancelButton, popupBox,
                                   world::PixelPoint{click.x, click.y})) {
            gameClient_.sendCommand(protocol::SleepMessage{false});
            voted_ = false;
            popupState_.hide();
            continue;
        }
        // El popup con la pregunta está encima de todo: sus opciones se
        // resuelven primero (coords de pantalla: es HUD), con las
        // MISMAS cajas que se dibujan. Sí (índice 0) confirma dormir; No
        // sale de la cama.
        if (popupState_.view() == game::PopupView::Question) {
            const int option =
                    promptLayout_.optionAt(popupBox, world::PixelPoint{click.x, click.y});
            if (option >= 0) {
                voted_ = option == 0;
                gameClient_.sendCommand(protocol::SleepMessage{voted_});
                if (voted_) {
                    popupState_.showWaiting(lastSleeping_, lastTotal_);
                } else {
                    popupState_.hide();
                }
                continue;
            }
        }
        // Modal: con el popup abierto el mundo está bloqueado; el resto
        // de los clicks no hace nada (se drenan para que no se disparen
        // en cascada al cerrar).
        if (popupState_.modal()) {
            continue;
        }
        // La cama. Caminar hasta ella lo decide EnterSelection; el popup
        // lo abre el server (ServerSleep) al pisarla. Si ya estoy adentro
        // (sin walkTo), el click pide dormir con Take, que la registra.
        const game::EnterClick enterClick = enterSelection_.clickAt(worldClick, state);
        if (enterClick.hit) {
            if (enterClick.walkTo.has_value()) {
                player_.walkTo(*enterClick.walkTo);
            } else if (enterClick.ask) {
                player_.interact();
            }
            continue;
        }
        const game::PickTarget clicked =
                picker_.pick(worldClick, state, motion_, iteration, scene_.cellPixelSize);
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
    bool interactive = isInteractive(target, state) || enterSelection_.hits(worldCursor, state);
    if (popupState_.modal()) {
        interactive = (popupState_.view() == game::PopupView::Question &&
                       promptLayout_.optionAt(popupBox, mousePoint) >= 0) ||
                      (popupState_.view() == game::PopupView::Waiting &&
                       popupState_.waitingCancelable() &&
                       promptLayout_.buttonAt(world::kPromptCancelButton, popupBox, mousePoint));
    }
    cursorPainter_.draw(cursorCatalog_,
                        interactive ? world::CursorKind::Interactive : world::CursorKind::Normal,
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
    renderer_.drawElementFront(scene_, scene_.elements[front->elementIndex], cells,
                               camera_.viewport(), front->firstRow, front->lastRow);
}

void PlaySession::changeScene(const std::string& room) {
    // El cambio en el negro de la transición: cambia QUÉ escena se
    // pinta (el dibujo es el mismo). La posición del personaje y todo
    // lo demás lo sigue mandando la fuente de estado por la vista.
    scene_ = world::SceneConfigLoader().loadFromFile(core::assetPath(core::kScenesFile), room);
    renderer_.prepareScene(scene_);
    enterSelection_.reindex(scene_, scene_.cellPixelSize);
    camera_.setWorldBounds(world::ScenePixelMapper(scene_.cellPixelSize).worldBounds(scene_.grid));
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
    inventoryHud_.drawPanel(panel, promptLayout_.grid.columns, promptLayout_.grid.rows);
    const world::PixelPoint mouse{input_.mousePosition().x, input_.mousePosition().y};
    promptPainter_.draw(popupState_, promptLayout_.optionAt(panel, mouse), panel);
}

void PlaySession::closeChat() {
    chatInput_.clear();
    input_.setTextMode(false);
    sdlContext_.stopTextInput();
}

void PlaySession::eraseLastCodepoint(std::string& text) {
    if (text.empty()) {
        return;
    }
    auto it = text.end();
    utf8::unchecked::prior(it);
    text.erase(it, text.end());
}

void PlaySession::updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                               int iteration) {
    if (view.entities.empty()) {
        return;
    }
    camera_.follow(motion.drawBoxFor(view.entities[game::kLocalPlayerIndex],
                                     game::kLocalPlayerIndex, iteration, scene_.cellPixelSize));
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
