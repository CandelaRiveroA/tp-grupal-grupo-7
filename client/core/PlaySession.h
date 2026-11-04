#pragma once

#include <map>

#include "core/StartupScreens.h"
#include "core/ToolUseSounds.h"
#include "core/WindowConfig.h"
#include "game/ActionResolver.h"
#include "game/DialogAnimator.h"
#include "game/FarmSelection.h"
#include "game/FootstepCue.h"
#include "game/ObjectPicker.h"
#include "game/ObtainedBannerQueue.h"
#include "game/PlayerController.h"
#include "game/ResourceAudioCue.h"
#include "game/ViewMotion.h"
#include "sdl/audio/AudioCatalog.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/context/SdlContext.h"
#include "sdl/input/InputManager.h"
#include "sdl/render/Camera.h"
#include "sdl/render/BuildPlacePainter.h"
#include "sdl/render/CursorPainter.h"
#include "sdl/render/DialogPainter.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/ObtainedBannerHud.h"
#include "sdl/render/Renderer.h"
#include "world/CursorCatalog.h"
#include "world/DialogCatalog.h"
#include "world/FarmCatalogs.h"
#include "world/InventoryLayout.h"

namespace game {
class GameClient;
struct LocalWorldView;
}

namespace world {
class Scenario;
class ScenePixelMapper;
class CharacterCatalog;
struct DigitCatalog;
struct InventoryLayout;
struct ObtainedBannerCatalog;
struct ResourceCatalog;
struct ToolOverlayCatalog;
}

namespace core {

// Una partida en curso: posee los objetos de la sesión de juego y
// orquesta cada frame (estado -> cámara -> pantallas de arranque ->
// intención del jugador -> audio -> dibujo). Application solo la
// construye y la corre; las decisiones viven en las clases que posee.
class PlaySession {
public:
    PlaySession(const WindowConfig& config, const world::Scenario& scenario,
                game::GameClient& gameClient,
                const world::CharacterCatalog& characters,
                const world::InventoryLayout& inventory,
                const world::ToolOverlayCatalog& toolOverlay,
                const world::ResourceCatalog& resources,
                const world::ObtainedBannerCatalog& banner,
                const world::DigitCatalog& digits,
                const world::CursorCatalog& cursors,
                const world::DialogCatalog& dialogs);

    // Corre el game loop hasta que se pida salir.
    void run();

private:
    void tick(int iteration);

    // Marcador del lugar de plantado sobre la celda que apunta el puntero.
    void drawPlantingMarker(const world::ScenePixelMapper& cells);

    // La cámara sigue el rectángulo dibujado del jugador local, no su
    // celda lógica: así también se centra durante la interpolación del paso.
    void updateCamera(const game::LocalWorldView& view,
                      const game::ViewMotion& motion, int iteration);

    // ¿El objetivo del pick es interactivo? Lo dice el estado (la fuente
    // decide); esto solo lo lee para elegir la variante del cursor.
    bool isInteractive(const game::PickTarget& target,
                       const game::LocalWorldView& view) const;

    const world::Scenario& scenario_;
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
    game::ObtainedBannerQueue bannerQueue_;
    sdl::InputManager input_;
    sdl::AudioPlayer audio_;

    StartupScreens screens_;
    game::ViewMotion motion_;
    game::FootstepCue footstepCue_;
    ToolUseSounds toolUseSounds_;
    game::ResourceAudioCue resourceAudioCue_;
    game::FarmSelection farmSelection_;
    game::ActionResolver actionResolver_;
    game::PlayerController player_;
    game::ObjectPicker picker_;
    sdl::BuildPlacePainter markerPainter_;
    world::CursorCatalog cursorCatalog_;
    sdl::CursorPainter cursorPainter_;
    game::DialogAnimator dialogAnimator_;
    world::DialogCatalog dialogCatalog_;
    sdl::DialogPainter dialogPainter_;

    // Sonido por cada evento de recurso confirmado. Es dato: se arma una
    // vez en el constructor, no por frame.
    std::map<game::ResourceAudioEvent, sdl::SoundId> resourceSounds_;
};

}  // namespace core
