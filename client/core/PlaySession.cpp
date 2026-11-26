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
#include "game/AreaProximity.h"
#include "game/GameClient.h"
#include "game/GameConstants.h"
#include "game/EnemyTestTrigger.h"
#include "game/LocalWorldView.h"
#include "sdl/log/Logger.h"
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
                         const world::NpcCatalog& npcs, const world::MarketCatalog& market,
                         const world::InventoryLayout& inventory,
                         const world::ToolOverlayCatalog& toolOverlay,
                         const world::ResourceCatalog& resources,
                         const world::ObtainedBannerCatalog& banner,
                         const world::DigitCatalog& digits, const world::CursorCatalog& cursors,
                         const world::DialogCatalog& dialogs, const world::FontCatalog& font,
                         const world::PromptLayout& prompt, const world::ChatLayout& chat,
                         const world::NoticeLayout& notice,
                         const world::NameplateLayout& nameplate,
                         const world::SettingsLayout& settings,
                         const world::HudOverlayCatalog& hudOverlays,
                         const world::HudOverlayCatalog& lighting,
                         const world::CharacterPalettes& palettes,
                         game::EnemyTestTrigger* enemyTestTrigger,
                         const std::string& worldPath,
                         std::string initialSceneId):
        sources_(std::string(), worldPath, core::assetPath(core::kOverlayFile)),
        sceneSwitcher_(sources_.byPriority(), std::move(initialSceneId), scenario.scene()),
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
        npcCatalog_(npcs),
        marketCatalog_(market),
        farm_(core::assetPath(core::kCropsFile), core::assetPath(core::kGroundFile),
              core::assetPath(core::kBuildPlaceFile)),
        inventory_(farm_.withCropContents(inventory)),
        sdlContext_(config),
        camera_(world::ScenePixelMapper(scenario.scene().cellPixelSize)
                        .worldBounds(scenario.scene().grid),
                sdlContext_.width(), sdlContext_.height()),
        renderer_(sdlContext_, characters, toolOverlay, resources, inventory_.content,
                  farm_.ground(), farm_.crops(), palettes),
        digitPainter_(sdlContext_, digits),
        inventoryHud_(sdlContext_, inventory_, digitPainter_),
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
        structureSelection_(scenario.scene(), scenario.scene().cellPixelSize),
        structureActions_{{world::StructureAction::Popup, &PlaySession::openStructurePopup}},
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
        buttonPainter_(sdlContext_, promptLayout_),
        promptPainter_(textRenderer_, promptLayout_, buttonPainter_),
        npcPopupPainter_(sdlContext_, textRenderer_),
        marketPopupPainter_(sdlContext_, textRenderer_),
        marketPopup_(market, inventory_),
        chatPainter_(sdlContext_, textRenderer_, chat),
        noticeQueue_(notice.durationTicks),
        noticePainter_(sdlContext_, textRenderer_, notice),
        nameplatePainter_(textRenderer_, nameplate),
        settingsLayout_(settings),
        settings_(settingsLayout_.defaultVolumes()),
        settingsPainter_(sdlContext_, textRenderer_, buttonPainter_, digitPainter_,
                         settingsLayout_),
        resourceSounds_{
                {game::ResourceAudioEvent::Hit, sdl::SoundId::MineralHit},
                {game::ResourceAudioEvent::Broke, sdl::SoundId::MineralCrack},
        },
        commandSounds_{
                {game::InputCommand::Kind::ToggleSettings, sdl::SoundId::Cancel},
                {game::InputCommand::Kind::ChatCancel, sdl::SoundId::Cancel},
        } {
    // El volumen elegido de cada categoría rige desde el primer sonido.
    audioMixer_.apply(settings_.volumes());
    renderer_.prepareScene(scene());
    renderer_.setNpcs(npcCatalog_.forRoom(sceneSwitcher_.id()));
    // Entradas del overlay que no coinciden con nada del mundo: un
    // aviso por cada una (el path dice cuál), no frena el juego.
    const sdl::Logger logger;
    for (const world::ConfigError& finding: sources_.findings()) {
        logger.warning(finding.what());
    }
}

const world::SceneDescription& PlaySession::scene() const { return sceneSwitcher_.scene(); }

bool PlaySession::run() {
    bool leaving = false;
    sdl::GameLoop loop(input_, [this, &loop, &leaving](int iteration) {
        tick(iteration);
        // Abandonar vuelve al lobby; cerrar la ventana o perder la conexión
        // terminan el programa, como siempre.
        leaving = settings_.takeLeaveRequest();
        if (leaving || !gameClient_.connected()) {
            loop.stop();
        }
    });
    loop.run();
    return leaving;
}

void PlaySession::tick(int iteration) {
    gameClient_.update(iteration);
    const game::LocalWorldView& state = gameClient_.worldState();
    const world::DialogData* thanks = dialogCatalog_.find(world::Indication::GiftThanks);
    giftThanks_.update(state.giftAcceptedPlayers,
                       thanks ? thanks->animation.durationTicks() : 0);
    // La presentación local del tick (desmayo incluido) es la que ven
    // el movimiento, el renderer y los diálogos.
    game::LocalWorldView presentation =
            presentationView(sceneSnapshot_.observe(state, sceneSwitcher_.id()));
    motion_.observe(presentation, iteration);
    renderer_.setNpcFrames(props_.frames(iteration));
    updateCamera(presentation, motion_, iteration);
    // El reloj del server decide la etapa del día y lo que muestra el HUD.
    dayPhase_.setClock(state.dayNumber, state.clockMinutes);
    dayPhase_.advance();
    if (chatCue_.update(state.incomingChatVersion)) {
        audio_.play(sdl::SoundId::NewMessage);
    }

    // Se terminó el día: se cierra todo y la pantalla se funde a negro y SE
    // QUEDA ahí con el popup del nuevo día hasta que despierten todos. Ahí
    // (ServerDay) cambia la escena (dormitorio), vuelve el fade y suena el gallo.
    // Va ANTES de la secuencia de inicio: un jugador que se une durante el
    // popup recibe el evento mientras todavía está en la intro.
    if (state.dayEnded) {
        voted_ = false;
        woke_ = false;
        dayEnding_ = true;
        npcPopup_ = nullptr;
        lastNpcId_.clear();
        marketPopup_.close();
        settings_.close();
        popupState_.hide();
        sceneTransition_.changeTo(sceneSwitcher_.id(), true);
        // El cambio de escena lo decide el server (ServerMapChange) al
        // despertar; no se fuerza un mapa local acá.
    }
    if (state.dayChanged) {
        dayEnding_ = false;
        // Lo que cambió en el sueño durante el fin de día ya es historia:
        // sin esto, el primer tick libre lo trataría como un cambio nuevo
        // y reabriría "Esperando al resto de jugadores...".
        lastSleeping_ = state.sleep.sleeping;
        lastTotal_ = state.sleep.total;
        audio_.play(sdl::SoundId::RoosterCrow);
        // Día nuevo: los stats simulados arrancan llenos (lo dirá el server).
        for (auto& entry : stats_) {
            entry.second.reset();
        }
    }

    const GamePhase phase = phaseForIteration(iteration);
    if (screens_.draw(phase, iteration, presentation, motion_, actionResolver_, dayPhase_.phase())) {
        return;
    }

    // La transición de escena manda: el "apagado y vuelta a prender".
    // En el negro cambian los datos; mientras está activa se dibuja el
    // mundo con el fade encima y NO hay input (las colas se drenan para
    // que nada se dispare al volver).
    // Con servidor, la escena la dice la fuente de estado (mapId): el
    // cambio entra por el mismo fade (si ya hay uno en curso, sigue).
    if (const std::optional<std::string> requested = sceneSwitcher_.changeFor(state.mapId)) {
        sceneTransition_.changeTo(*requested);
    }
    if (state.dayChanged) {
        // Primero actualizar el destino recibido del servidor; después
        // levantar el negro, para no volver a dibujar el mapa anterior.
        sceneTransition_.release();
    }
    sceneTransition_.tick(iteration);
    if (sceneTransition_.swapDue()) {
        changeScene(sceneTransition_.scene());
        sceneTransition_.consumeSwap();
        presentation = presentationView(sceneSnapshot_.observe(state, sceneSwitcher_.id()));
        // Un traslado no interpola el recorrido desde las coordenadas
        // del mapa anterior hasta la cama del mapa nuevo.
        motion_.observe(game::LocalWorldView{}, iteration);
        motion_.observe(presentation, iteration);
        updateCamera(presentation, motion_, iteration);
    }
    // La caja del popup (figura del inventario centrada, en la grilla
    // que digan sus datos): donde se dibuja y donde se tocan sus cosas.
    const world::PixelBox popupBox =
            inventory_.centeredBox(sdlContext_.width(), sdlContext_.height(),
                                   promptLayout_.grid.columns, promptLayout_.grid.rows);
    if (sceneTransition_.active()) {
        input_.takeCommands();
        // Sin popup de configuración durante la transición.
        settings_.close();
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
            renderer_.drawWorld(scene(), presentation, motion_, actionResolver_, iteration,
                                camera_.viewport(), dayPhase_.phase());
            lightingPainter_.draw({}, dayPhase_.nightFactor());
            drawNameplates(presentation, iteration);
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
    if (!dayEnding_ &&
        (state.sleep.sleeping != lastSleeping_ || state.sleep.total != lastTotal_)) {
        lastSleeping_ = state.sleep.sleeping;
        lastTotal_ = state.sleep.total;
        npcPopup_ = nullptr;
        lastNpcId_.clear();
        marketPopup_.close();
        settings_.close();
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

    // Llegó un pedido de regalo (ServerGiftRequest): abre el popup del
    // receptor con el nombre del emisor y guarda a quién se le contesta.
    if (state.giftRequest.has_value()) {
        pendingGiftSenderId_ = state.giftRequest->senderId;
        settings_.close();
        popupState_.showGiftReceived(state.giftRequest->senderName);
    }

    // El receptor contestó MI oferta (acepte o rechace): se cierra la
    // espera. Solo si sigue abierta: la respuesta puede llegar tarde,
    // después de cancelar a mano.
    if (state.giftAnswered && popupState_.view() == game::PopupView::GiftWaiting) {
        popupState_.hide();
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

    openMarketFromView(state);
    const std::vector<game::InputCommand> commands = input_.takeCommands();
    std::vector<game::InputCommand> playerCommands;
    // ANDAMIAJE TEMPORAL (prueba): las teclas G/F/H/K disparan acciones
    // de los enemigos en el mundo simulado (si hay), J/L mueven/borran
    // sus proyectiles y V hace que el Skull ataque con el fuego verde.
    // El server real pondrá la acción/posición directo en la vista y
    // esto se saca.
    for (const game::InputCommand& command: commands) {
        const auto commandSound = commandSounds_.find(command.kind);
        if (commandSound != commandSounds_.end()) {
            audio_.play(commandSound->second);
        }
        // Enter con texto manda y deja el chat ABIERTO; Enter con la línea
        // vacía lo cierra, igual que ESC. Pasan aunque el popup sea modal.
        if (command.kind == game::InputCommand::Kind::ChatSubmit) {
            if (chatInput_.empty()) {
                closeChat();
            } else {
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
        if (command.kind == game::InputCommand::Kind::NextMarketSet) {
            // DEMO (tecla N): rota el estilo del último comerciante y, si
            // su catálogo está abierto, lo reabre con el estilo nuevo.
            if (!lastNpcId_.empty() && marketCatalog_.hasMerchant(lastNpcId_)) {
                marketCycle_.next(marketCatalog_.defaultSetCount(lastNpcId_));
                if (marketPopup_.isOpen()) {
                    marketPopup_.close();
                    openMarketOfSpokenNpc();
                }
            }
            continue;
        }
        if (command.kind == game::InputCommand::Kind::OpenCatalog) {
            // DEMO (tecla B): catálogo de prueba, sin pasar por el NPC.
            toggleDemoCatalog();
            continue;
        }
        if (command.kind == game::InputCommand::Kind::ToggleSettings) {
            toggleSettings();
            continue;
        }
        // Con el popup de un NPC (o el de configuración) abierto el mundo
        // está bloqueado: el click lo cierra y el resto de las intenciones
        // se descartan.
        if (npcPopup_ != nullptr || marketPopup_.isOpen() || settings_.modal()) {
            continue;
        }
        if (popupState_.modal()) {
            // El popup manda: el resto de las intenciones se descartan
            // (no se encolan para cuando cierre).
            continue;
        }
        if (command.kind == game::InputCommand::Kind::OpenCatalog) {
            // El catálogo real se pide por /consultar_catalogo en el chat.
            continue;
        }
        if (command.kind == game::InputCommand::Kind::ToggleMarket) {
            // Cambio de escenario local: mercado ↔ dormitorio.
            sceneTransition_.changeTo(sceneSwitcher_.id() == core::kMarketRoom ? core::kInitialRoom
                                                                        : core::kMarketRoom);
            continue;
        }
        if (command.kind == game::InputCommand::Kind::CycleMap) {
            // DEMO: escena siguiente (mapas del editor y salas).
            sceneTransition_.changeTo(sceneSwitcher_.nextId());
            continue;
        }
        playerCommands.push_back(command);
    }
    // Mientras el popup es modal no hay input manual: la auto-caminata
    // en curso (el "meterse") termina su paso y después el personaje
    // queda quieto hasta que se responda.
    const game::PlayerEvents events =
            player_.tick(iteration,
                         popupState_.modal() || npcPopup_ != nullptr || marketPopup_.isOpen() ||
                                         settings_.modal() ?
                                 std::nullopt :
                                 pressed,
                         playerCommands);
    if (events.toolSwitched) {
        audio_.play(sdl::SoundId::ToolSwitch);
    }

    // Presentación local del campo (saco en la cabeza y lugar de
    // plantado): sobre la selección fresca de este tick, y la celda que
    // apunta el puntero (el Viewport traduce a mundo).
    const world::ScenePixelMapper cells(scene().cellPixelSize);
    const world::PixelPoint worldCursor = camera_.viewport().toWorld(
            world::PixelPoint{input_.mousePosition().x, input_.mousePosition().y});
    farmSelection_.observe(state, player_.activeSlot(), cells.toCell(worldCursor));
    // Dónde está cada NPC lo manda la fuente de estado (vista); sin eso,
    // valen las celdas por defecto de npcs.toml.
    const world::NpcCells npcCells = npcPlacements_.cellsFrom(state, scene());

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
    noticeQueue_.observe(state.notices);

    // El mundo se dibuja con la vista "puesta": si el jugador está
    // metido en un elemento, en su pose (presentación local; ver
    // EnterSelection). Encima, la capa frontal que lo tapa.
    renderer_.drawWorld(scene(), presentation, motion_, actionResolver_, iteration,
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
                motion_.drawBoxFor(entity, i, iteration, scene().cellPixelSize);
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
    // Nombres bajo los pies: siguen a la cámara pero van después de la luz
    // nocturna, así no se oscurecen.
    drawNameplates(presentation, iteration);
    inventoryHud_.draw(state.inventory, player_.activeSlot());
    bannerHud_.draw(bannerQueue_.active());
    noticePainter_.draw(noticeQueue_.active());
    noticeQueue_.tick();
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
    // Valores crudos de las etiquetas: el reloj y la plata (la manda la fuente).
    std::map<std::string, int> hudValues = dayPhase_.clockValues();
    hudValues[game::kHudGoldKey] = state.gold;
    hudOverlayPainter_.drawLabels(hudValues);
    // Abierto: historial + tipeo (el cursor parpadea con el tick).
    // Cerrado: el preview atenuado, que se apaga solo.
    if (input_.chatOpen()) {
        chatPainter_.drawOpen(chatInput_, state.chatLog,
                              (iteration / core::kChatBlinkTicks) % 2 == 0);
    } else if (iteration < chatPreviewUntilTick_) {
        chatPainter_.drawPreview(state.chatLog);
    }
    drawPopup(popupBox);
    drawNpcPopup();
    drawMarketCatalog();
    dragSettingsSlider();
    drawSettings();

    // El puntero: la variante la decide el estado de juego (si hay algo
    // interactivo bajo el cursor); el painter solo dibuja.
    const game::PickTarget target =
            picker_.pick(worldCursor, state, motion_, iteration, scene().cellPixelSize);

    // El click sobre algo interactivo manda la intención de tomarlo; el
    // resto (piso, rocas, nada) cae al uso de herramienta hacia esa
    // celda. La fuente decide qué pasa en ambos casos. Antes que todo:
    // el click sobre un elemento entrable lo mete/saca (presentación
    // local) y se consume ahí.
    for (const game::PointerClick& click: input_.takeClicks()) {
        // Con la configuración abierta solo cuentan sus controles.
        if (settings_.modal()) {
            handleSettingsClick(click);
            continue;
        }
        const world::PixelPoint worldClick =
                camera_.viewport().toWorld(world::PixelPoint{click.x, click.y});
        // El popup de un NPC o el del catálogo se cierra con cualquier click.
        if (npcPopup_ != nullptr || marketPopup_.isOpen()) {
            npcPopup_ = nullptr;
            marketPopup_.close();
            continue;
        }
        const world::PixelPoint screenClick{click.x, click.y};
        const game::PopupView view = popupState_.view();
        const world::PromptCopy& prompt = promptLayout_.prompt(popupState_.promptId());

        // El botón de cancelar de los popups de espera (sueño y regalo)
        // reutiliza el botón del layout. Cancelar el regalo solo cierra a
        // nivel dibujo: la consecuencia la decidirá el server.
        const bool cancelShown = view == game::PopupView::GiftWaiting ||
                                 (view == game::PopupView::Waiting &&
                                  popupState_.waitingCancelable());
        if (cancelShown &&
            !prompt.button.empty() &&
            promptLayout_.buttonAt(prompt.button, popupBox, screenClick)) {
            if (view == game::PopupView::Waiting) {
                gameClient_.sendCommand(protocol::SleepMessage{false});
                voted_ = false;
            }
            popupState_.hide();
            continue;
        }

        // Las preguntas (sueño, ofrecer regalo, aceptar regalo) usan las
        // MISMAS cajas que se dibujan.
        if (view != game::PopupView::Hidden && !prompt.options.empty()) {
            const int option = promptLayout_.optionAt(
                    popupBox, screenClick, prompt);
            if (option >= 0) {
                if (view == game::PopupView::Question) {
                    voted_ = option == 0;
                    gameClient_.sendCommand(protocol::SleepMessage{voted_});
                    if (voted_) {
                        popupState_.showWaiting(lastSleeping_, lastTotal_);
                    } else {
                        popupState_.hide();
                    }
                } else if (view == game::PopupView::GiftOffer) {
                    // Sí manda la intención (Take con la mano, igual que
                    // cualquier interact): el server dispara el pedido de
                    // regalo hacia el otro jugador. No: solo cierra.
                    if (option == 0) {
                        player_.interact();
                        popupState_.showGiftWaiting();
                    } else {
                        popupState_.hide();
                    }
                } else if (view == game::PopupView::GiftReceived) {
                    // La respuesta va al emisor del pedido (si el popup no
                    // lo abrió la red, no hay a quién contestarle).
                    popupState_.hide();
                    if (pendingGiftSenderId_.has_value()) {
                        gameClient_.sendCommand(protocol::GiftResponseMessage{
                                *pendingGiftSenderId_, option == 0});
                        pendingGiftSenderId_.reset();
                    }
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
        // Un NPC: abre su popup y consume el click.
        if (const world::NpcDef* npc = npcCatalog_.npcAt(sceneSwitcher_.id(), worldClick,
                                                         scene().cellPixelSize, npcCells);
            npc != nullptr) {
            if (!npcPlacements_.canInteractWith(*npc, state, npcCells)) {
                continue;
            }
            if (!npc->hasPopup()) {
                // Objeto animado (cofre): el click lo abre o lo cierra.
                props_.toggle(npc->id, static_cast<int>(npc->body.frames.size()),
                              npc->body.frameTicks, iteration);
                continue;
            }
            npcPopup_ = npc;
            // Conserva el comerciante seleccionado para el flujo local del chat.
            lastNpcId_ = npc->id;
            if (npc->id == "pierre" || npc->id == "marnie" ||
                npc->id == "clint" || npc->id == "robin") {
                player_.interact();
            }
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
        // Un edificio del editor: lejos no hace nada (como la cama),
        // y en ambos casos el click se consume.
        const game::StructureClick structureClick = structureSelection_.clickAt(worldClick, state);
        if (structureClick.hit) {
            if (structureClick.inRange) {
                runStructureAction(*structureClick.structure);
            }
            continue;
        }
        const game::PickTarget clicked =
                picker_.pick(worldClick, state, motion_, iteration, scene().cellPixelSize);
        if (clicked.kind == game::PickTarget::Kind::Furniture) {
            if (isInteractive(clicked, state)) {
                const game::ViewFurniture& furniture = state.placedFurniture[clicked.index];
                const game::ViewEntity& local = state.entities[game::kLocalPlayerIndex];
                if (local.cellX == furniture.cellX && local.cellY == furniture.cellY) {
                    player_.interact();
                } else {
                    player_.walkTo(world::GridCell{furniture.cellX, furniture.cellY});
                }
            }
            continue;
        }
        // Otro jugador: ofrecer un regalo. El click no dispara la
        // herramienta ni la interacción.
        if (isGiftTarget(clicked, state)) {
            popupState_.showGiftOffer();
            continue;
        }
        if (clicked.kind != game::PickTarget::Kind::None && isInteractive(clicked, state)) {
            player_.interact();
            continue;
        }
        player_.useToward(cells.toCell(worldClick), pressed);
    }

    // El puntero: mientras el popup es modal, el mundo está bloqueado y
    // solo es apretable lo del popup (las opciones o el botón de su
    // copia). Fuera del popup, otro jugador muestra el cursor de regalo.
    const world::PixelPoint mousePoint{input_.mousePosition().x, input_.mousePosition().y};
    world::CursorKind cursorKind = world::CursorKind::Normal;
    // Con el popup de un NPC o del catálogo abierto el mundo está bloqueado.
    const bool worldBlocked = npcPopup_ != nullptr || marketPopup_.isOpen() || settings_.modal();
    const world::NpcDef* hoveredNpc = npcCatalog_.npcAt(
            sceneSwitcher_.id(), worldCursor, scene().cellPixelSize, npcCells);
    if (settings_.modal()) {
        cursorKind = overSettingsControl(mousePoint) ? world::CursorKind::Interactive :
                                                       world::CursorKind::Normal;
    } else if (popupState_.modal()) {
        const world::PromptCopy& prompt = promptLayout_.prompt(popupState_.promptId());
        bool overPopup = false;
        if (!prompt.options.empty()) {
            overPopup = promptLayout_.optionAt(
                    popupBox, mousePoint, prompt) >= 0;
        } else if (!prompt.button.empty() &&
                   (popupState_.view() != game::PopupView::Waiting ||
                    popupState_.waitingCancelable())) {
            overPopup = promptLayout_.buttonAt(prompt.button, popupBox, mousePoint);
        }
        cursorKind = overPopup ? world::CursorKind::Interactive : world::CursorKind::Normal;
    } else if (!worldBlocked && isGiftTarget(target, state)) {
        cursorKind = world::CursorKind::Gift;
    } else if (!worldBlocked &&
               (isInteractive(target, state) || enterSelection_.hits(worldCursor, state) ||
                structureSelection_.hits(worldCursor, state) ||
                (hoveredNpc != nullptr &&
                 npcPlacements_.canInteractWith(*hoveredNpc, state, npcCells)))) {
        cursorKind = world::CursorKind::Interactive;
    }
    cursorPainter_.draw(cursorCatalog_, cursorKind, input_.mousePosition());
    sdlContext_.present();
}

game::LocalWorldView PlaySession::presentationView(const game::LocalWorldView& view) const {
    // Primero la pose de "meterse" (EnterSelection) y encima el
    // agradecimiento del regalo recibido por red.
    game::LocalWorldView display = enterSelection_.displayView(view);
    giftThanks_.apply(display);
    return display;
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
    renderer_.drawElementFront(scene(), scene().elements[front->elementIndex], cells,
                               camera_.viewport(), front->firstRow, front->lastRow);
}

void PlaySession::runStructureAction(const world::SceneStructure& structure) {
    if (structure.action == world::StructureAction::EnterMap && structure.door.has_value()) {
        player_.walkTo(*structure.door);
        return;
    }
    const auto action = structureActions_.find(structure.action);
    if (action != structureActions_.end()) {
        (this->*(action->second))(structure.target);
    }
}

void PlaySession::openStructurePopup(const std::string& npcId) {
    npcPopup_ = npcCatalog_.find(npcId);
}


void PlaySession::changeScene(const std::string& room) {
    // El cambio en el negro de la transición: cambia QUÉ escena se
    // pinta (el dibujo es el mismo). La posición del personaje y todo
    // lo demás lo sigue mandando la fuente de estado por la vista. De
    // dónde sale la escena lo resuelve SceneSwitcher; esto solo aplica
    // el resultado a quienes la usan.
    sceneSwitcher_.switchTo(room);
    const world::SceneDescription& current = sceneSwitcher_.scene();
    npcPopup_ = nullptr;
    lastNpcId_.clear();
    marketPopup_.close();
    renderer_.prepareScene(current);
    renderer_.setNpcs(npcCatalog_.forRoom(room));
    enterSelection_.reindex(current, current.cellPixelSize);
    structureSelection_.reindex(current, current.cellPixelSize);
    camera_.setWorldBounds(
            world::ScenePixelMapper(current.cellPixelSize).worldBounds(current.grid));
}

void PlaySession::drawNameplates(const game::LocalWorldView& view, const int iteration) {
    // QUÉ nombre lo dice la vista; dónde y de qué color, NameplateLayout.
    // La caja de cada entidad es la MISMA que usan la cámara y los diálogos.
    const int hour = dayPhase_.clockValues().at(game::kClockHourKey);
    for (std::size_t i = 0; i < view.entities.size(); ++i) {
        const game::ViewEntity& entity = view.entities[i];
        if (entity.displayName.empty()) {
            continue;
        }
        nameplatePainter_.draw(entity.displayName,
                               motion_.drawBoxFor(entity, i, iteration, scene().cellPixelSize),
                               camera_.viewport(), hour);
    }
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
    const world::PromptCopy& prompt = promptLayout_.prompt(popupState_.promptId());
    const int hovered = prompt.options.empty()
            ? -1
            : promptLayout_.optionAt(panel, mouse, prompt);
    promptPainter_.draw(popupState_, hovered, panel);
}

void PlaySession::toggleSettings() {
    // Cerrar siempre se puede; abrir no si otro popup ya bloquea el mundo.
    const bool otherPopup = npcPopup_ != nullptr || marketPopup_.isOpen() || popupState_.modal();
    if (!settings_.isOpen() && otherPopup) {
        return;
    }
    settings_.toggle();
}

world::PixelBox PlaySession::settingsPanel() const {
    return inventory_.centeredBox(sdlContext_.width(), sdlContext_.height(),
                                  settingsLayout_.grid.columns, settingsLayout_.grid.rows);
}

void PlaySession::drawSettings() {
    if (!settings_.isOpen()) {
        return;
    }
    const world::PixelBox panel = settingsPanel();
    inventoryHud_.drawPanel(panel, settingsLayout_.grid.columns, settingsLayout_.grid.rows);
    settingsPainter_.draw(settings_, panel);
}

bool PlaySession::overSettingsControl(const world::PixelPoint& point) const {
    const world::SettingsBoxes boxes = settingsLayout_.boxes(settingsPanel());
    return settingsLayout_.onQrButton(boxes, point) ||
           settingsLayout_.onLeaveButton(boxes, point) ||
           settingsLayout_.sliderAt(boxes, point).has_value();
}

void PlaySession::handleSettingsClick(const game::PointerClick& click) {
    const world::SettingsBoxes boxes = settingsLayout_.boxes(settingsPanel());
    const world::PixelPoint point{click.x, click.y};
    if (settingsLayout_.onQrButton(boxes, point)) {
        settings_.requestQr();
        return;
    }
    if (settingsLayout_.onLeaveButton(boxes, point)) {
        settings_.requestLeave();
        return;
    }
    if (const std::optional<world::SoundCategory> category =
                settingsLayout_.sliderAt(boxes, point)) {
        settings_.beginDrag(*category);
        setVolumeFromPointer(*category, click.x);
    }
}

void PlaySession::dragSettingsSlider() {
    const std::optional<world::SoundCategory>& category = settings_.dragging();
    if (!category.has_value()) {
        return;
    }
    if (!input_.pointerHeld()) {
        settings_.endDrag();
        return;
    }
    setVolumeFromPointer(*category, input_.mousePosition().x);
}

void PlaySession::setVolumeFromPointer(const world::SoundCategory category, const int x) {
    const world::SettingsBoxes boxes = settingsLayout_.boxes(settingsPanel());
    for (const world::VolumeRowBoxes& row: boxes.volume) {
        if (row.category == category) {
            settings_.setVolume(category, settingsLayout_.slider.percentAt(row.track, x));
        }
    }
    audioMixer_.apply(settings_.volumes());
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

void PlaySession::drawNpcPopup() {
    // Mismo panel que el popup del inventario, con la grilla y el
    // contenido que declara el NPC.
    if (npcPopup_ == nullptr) {
        return;
    }
    const world::GridSpan grid = npcPopup_->popup.grid;
    const world::PixelBox panel = inventory_.centeredBox(
            sdlContext_.width(), sdlContext_.height(), grid.columns, grid.rows);
    inventoryHud_.drawPanel(panel, grid.columns, grid.rows);
    npcPopupPainter_.draw(*npcPopup_, panel);
}

bool PlaySession::canInteractWithMerchant(const std::string& id,
                                         const game::LocalWorldView& view) const {
    const world::NpcDef* npc = npcCatalog_.find(id);
    return npc != nullptr && npc->room == sceneSwitcher_.id() &&
           npcPlacements_.canInteractWith(*npc, view, npcPlacements_.cellsFrom(view, scene()));
}

void PlaySession::openMarketOfSpokenNpc() {
    // El catálogo es el del último comerciante con el que se habló; sin
    // comerciante (o si el NPC no tiene catálogo) no abre nada.
    if (lastNpcId_.empty() || !marketCatalog_.hasMerchant(lastNpcId_) ||
        !canInteractWithMerchant(lastNpcId_, gameClient_.worldState())) {
        return;
    }
    const int setCount = marketCatalog_.defaultSetCount(lastNpcId_);
    marketPopup_.open(world::MarketOffer{
            lastNpcId_,
            marketCatalog_.defaultEntries(lastNpcId_, marketCycle_.current() % setCount)});
}

void PlaySession::toggleDemoCatalog() {
    if (marketPopup_.isOpen()) {
        marketPopup_.close();
        return;
    }
    if (npcPopup_ != nullptr || popupState_.modal() ||
        !canInteractWithMerchant(core::kDemoCatalogMerchant, gameClient_.worldState())) {
        return;
    }
    marketPopup_.open(world::MarketOffer{
            core::kDemoCatalogMerchant,
            marketCatalog_.defaultEntries(core::kDemoCatalogMerchant, 0)});
}

void PlaySession::openMarketFromView(const game::LocalWorldView& view) {
    // El catálogo que pide la fuente de estado. Abre el popup (cerrando el de
    // un NPC si lo hubiera) y reemplaza la oferta abierta si es de otro
    // comerciante. Se ignora con un popup modal.
    if (!view.marketOffer.has_value() || popupState_.modal() ||
        !canInteractWithMerchant(view.marketOffer->merchantId, view)) {
        return;
    }
    npcPopup_ = nullptr;
    if (marketPopup_.isOpen()) {
        if (marketPopup_.merchantId() == view.marketOffer->merchantId) {
            return;
        }
        marketPopup_.close();
    }
    marketPopup_.open(*view.marketOffer);
}


void PlaySession::drawMarketCatalog() {
    if (!marketPopup_.isOpen()) {
        return;
    }
    const world::GridSpan grid = marketPopup_.panelGrid();
    const world::PixelBox panel = marketPopup_.panel(sdlContext_.width(), sdlContext_.height());
    inventoryHud_.drawPanel(panel, grid.columns, grid.rows);
    marketPopupPainter_.draw(marketCatalog_.style(), marketPopup_.sections(), panel,
                             inventory_.content);
}

void PlaySession::updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                               int iteration) {
    if (view.entities.empty()) {
        return;
    }
    camera_.follow(motion.drawBoxFor(view.entities[game::kLocalPlayerIndex],
                                     game::kLocalPlayerIndex, iteration, scene().cellPixelSize));
}

bool PlaySession::isInteractive(const game::PickTarget& target,
                                const game::LocalWorldView& view) const {
    // El estado dice qué es interactivo; esto solo lo lee.
    if (target.kind == game::PickTarget::Kind::Furniture) {
        if (view.entities.empty()) {
            return false;
        }
        const game::ViewFurniture& furniture = view.placedFurniture[target.index];
        const world::SpriteVisual* visual = inventory_.content.find(furniture.contentId);
        if (furniture.contentId != "bed" || visual == nullptr) {
            return false;
        }
        const game::ViewEntity& local = view.entities[game::kLocalPlayerIndex];
        return game::AreaProximity(game::kInteractRing).around(
                world::GridCell{furniture.cellX, furniture.cellY - visual->rows + 1},
                world::GridSpan{visual->cols, visual->rows}, local.cellX, local.cellY);
    }
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

bool PlaySession::isGiftTarget(const game::PickTarget& target,
                               const game::LocalWorldView& view) const {
    // "Otro humano como nosotros, pero NO nosotros": mismo tipo Player y
    // un índice distinto del local. Lo decide la vista, no el render.
    if (target.kind != game::PickTarget::Kind::Entity ||
        target.index == game::kLocalPlayerIndex ||
        target.index >= view.entities.size()) {
        return false;
    }
    if (view.entities[target.index].kind != world::CreatureKind::Player) {
        return false;
    }
    // Solo se ofrece con algo transferible en el casillero activo: lo dice
    // el servidor (slot.tradable); con una herramienta o el casillero vacío
    // no se arma el pedido y el emisor quedaría esperando en vano.
    const int slot = player_.activeSlot();
    const auto held = std::find_if(view.inventory.begin(), view.inventory.end(),
                                   [slot](const game::ViewInventorySlot& inventorySlot) {
                                       return inventorySlot.slotIndex == slot;
                                   });
    return held != view.inventory.end() && held->tradable;
}

}  // namespace core
