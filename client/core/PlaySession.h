#pragma once

#include <map>
#include <string>

#include "core/StartupScreens.h"
#include "core/ToolUseSounds.h"
#include "core/WindowConfig.h"
#include "game/ActionResolver.h"
#include "game/DayPhase.h"
#include "game/DialogAnimator.h"
#include "game/EnterSelection.h"
#include "game/FarmSelection.h"
#include "game/FootstepCue.h"
#include "game/FractionStat.h"
#include "game/InputCommand.h"
#include "game/ObjectPicker.h"
#include "game/ObtainedBannerQueue.h"
#include "game/PlayerController.h"
#include "game/PopupState.h"
#include "game/ResourceAudioCue.h"
#include "game/SceneTransition.h"
#include "game/ViewMotion.h"
#include "sdl/audio/AudioCatalog.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/context/SdlContext.h"
#include "sdl/input/InputManager.h"
#include "sdl/render/BuildPlacePainter.h"
#include "sdl/render/Camera.h"
#include "sdl/render/CursorPainter.h"
#include "sdl/render/DialogPainter.h"
#include "sdl/render/HudOverlayPainter.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/ObtainedBannerHud.h"
#include "sdl/render/PromptPainter.h"
#include "sdl/render/Renderer.h"
#include "sdl/render/TextRenderer.h"
#include "world/CursorCatalog.h"
#include "world/DialogCatalog.h"
#include "world/FarmCatalogs.h"
#include "world/InventoryLayout.h"
#include "world/PromptLayout.h"
#include "world/SceneDescription.h"

namespace game {
class GameClient;
struct LocalWorldView;
}  // namespace game

namespace world {
class Scenario;
class ScenePixelMapper;
class CharacterCatalog;
struct DigitCatalog;
struct InventoryLayout;
struct ObtainedBannerCatalog;
struct ResourceCatalog;
struct ToolOverlayCatalog;
}  // namespace world

namespace core {

// Una partida en curso: posee los objetos de la sesión de juego y
// orquesta cada frame (estado -> cámara -> pantallas de arranque ->
// intención del jugador -> audio -> dibujo). Application solo la
// construye y la corre; las decisiones viven en las clases que posee.
class PlaySession {
public:
    PlaySession(const WindowConfig& config, const world::Scenario& scenario,
                game::GameClient& gameClient, const world::CharacterCatalog& characters,
                const world::InventoryLayout& inventory,
                const world::ToolOverlayCatalog& toolOverlay,
                const world::ResourceCatalog& resources, const world::ObtainedBannerCatalog& banner,
                const world::DigitCatalog& digits, const world::CursorCatalog& cursors,
                const world::DialogCatalog& dialogs, const world::FontCatalog& font,
                const world::PromptLayout& prompt, const world::HudOverlayCatalog& hudOverlays,
                const world::HudOverlayCatalog& lighting);

    // Corre el game loop hasta que se pida salir.
    void run();

private:
    void tick(int iteration);

    // Marcador del lugar de plantado sobre la celda que apunta el puntero.
    void drawPlantingMarker(const world::ScenePixelMapper& cells);

    // Capa frontal del elemento donde está metido el jugador (lo que lo
    // tapa). QUÉ dibujar lo decidió EnterSelection; esto solo lo pinta,
    // sobre las entidades y bajo el HUD.
    void drawEnteredFront(const game::LocalWorldView& view, const world::ScenePixelMapper& cells);

    // El popup del HUD con la figura vacía del inventario y su
    // contenido (la pregunta o el avance). El cuándo lo dice
    // PopupState, el dónde InventoryLayout y el cómo los painters;
    // esto solo compone la llamada.
    void drawPopup(const world::PixelBox& panel);

    // Cambia la escena actual (en el negro de la transición): carga la
    // sala pedida, reconstruye el índice del renderer y la cámara. El
    // dibujo no cambia: solo QUÉ escena se pinta.
    void changeScene(const std::string& room);

    // La cámara sigue el rectángulo dibujado del jugador local, no su
    // celda lógica: así también se centra durante la interpolación del paso.
    void updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                      int iteration);

    // ¿El objetivo del pick es interactivo? Lo dice el estado (la fuente
    // decide); esto solo lo lee para elegir la variante del cursor.
    bool isInteractive(const game::PickTarget& target, const game::LocalWorldView& view) const;

    // La escena actual: intercambiable al cambiar de escenario (la
    // ventana la sigue dando el Scenario).
    world::SceneDescription scene_;
    game::GameClient& gameClient_;
    const world::CharacterCatalog& characters_;

    // El campo (cultivos, terreno, lugar de plantado) y el inventario
    // con los sacos y la cosecha ya registrados. Van ANTES de todo lo
    // que los consume: el HUD y el renderer guardan lo que reciben.
    world::FarmCatalogs farm_;
    world::InventoryLayout inventory_;

    sdl::SdlContext sdlContext_;
    sdl::Camera camera_;
    sdl::Renderer renderer_;
    sdl::InventoryHud inventoryHud_;
    sdl::ObtainedBannerHud bannerHud_;
    // La herramienta única de texto: la comparten el popup y los HUD
    // (etiquetas de hud.toml), por eso se declara antes que ellos.
    sdl::TextRenderer textRenderer_;
    sdl::HudOverlayPainter hudOverlayPainter_;
    // Luz nocturna: mismo painter que el HUD fijo, con otro catálogo
    // (ui/lighting.toml) y dibujado antes de los HUD, modulado por la
    // intensidad de DayPhase.
    sdl::HudOverlayPainter lightingPainter_;
    game::ObtainedBannerQueue bannerQueue_;
    sdl::InputManager input_;
    sdl::AudioPlayer audio_;

    StartupScreens screens_;
    game::ViewMotion motion_;
    game::FootstepCue footstepCue_;
    ToolUseSounds toolUseSounds_;
    game::ResourceAudioCue resourceAudioCue_;
    game::FarmSelection farmSelection_;
    game::EnterSelection enterSelection_;
    game::PopupState popupState_;
    // Sueño: último aviso recibido y si el jugador ya votó Sí en esta noche.
    int lastSleeping_ = 0;
    int lastTotal_ = 0;
    bool voted_ = false;
    // Día terminado: ya apretó OK y espera a que despierten los demás.
    bool woke_ = false;
    game::SceneTransition sceneTransition_;
    // El momento del día (filtra los elementos `when` y gradúa la luz) y
    // el reloj del HUD. Entrada: el reloj del server (setClock en cada
    // tick).
    game::DayPhase dayPhase_;
    // Stats simulados del jugador: hoy los simula el cliente y el server
    // los reemplazará. `stats_` es estado; `statSpends_` es la regla de
    // qué comando gasta cuál. La traducción a las claves del HUD queda en
    // el borde de dibujo, no en el estado.
    struct StatSpend {
        game::StatKind stat;
        double cost = 0.0;
    };
    std::map<game::StatKind, game::FractionStat> stats_;
    std::map<game::InputCommand::Kind, StatSpend> statSpends_;
    game::ActionResolver actionResolver_;
    game::PlayerController player_;
    game::ObjectPicker picker_;
    sdl::BuildPlacePainter markerPainter_;
    world::CursorCatalog cursorCatalog_;
    sdl::CursorPainter cursorPainter_;
    game::DialogAnimator dialogAnimator_;
    world::DialogCatalog dialogCatalog_;
    sdl::DialogPainter dialogPainter_;
    world::PromptLayout promptLayout_;
    sdl::PromptPainter promptPainter_;

    // Sonido por cada evento de recurso confirmado. Es dato: se arma una
    // vez en el constructor, no por frame.
    std::map<game::ResourceAudioEvent, sdl::SoundId> resourceSounds_;
};

}  // namespace core
