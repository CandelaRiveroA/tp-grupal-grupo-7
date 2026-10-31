#pragma once

#include <map>

#include "core/StartupScreens.h"
#include "core/WindowConfig.h"
#include "game/ActionResolver.h"
#include "game/FootstepCue.h"
#include "game/ObtainedBannerQueue.h"
#include "game/PlayerController.h"
#include "game/ResourceAudioCue.h"
#include "game/ViewMotion.h"
#include "sdl/audio/AudioCatalog.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/context/SdlContext.h"
#include "sdl/input/InputManager.h"
#include "sdl/render/Camera.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/ObtainedBannerHud.h"
#include "sdl/render/Renderer.h"

namespace game {
class GameClient;
struct LocalWorldView;
}

namespace world {
class Scenario;
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
                const world::DigitCatalog& digits);

    // Corre el game loop hasta que se pida salir.
    void run();

private:
    void tick(int iteration);

    // La cámara sigue el rectángulo dibujado del jugador local, no su
    // celda lógica: así también se centra durante la interpolación del paso.
    void updateCamera(const game::LocalWorldView& view,
                      const game::ViewMotion& motion, int iteration);

    const world::Scenario& scenario_;
    game::GameClient& gameClient_;
    const world::CharacterCatalog& characters_;

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
    game::ResourceAudioCue resourceAudioCue_;
    game::ActionResolver actionResolver_;
    game::PlayerController player_;

    // Sonido por cada evento de recurso confirmado. Es dato: se arma una
    // vez en el constructor, no por frame.
    std::map<game::ResourceAudioEvent, sdl::SoundId> resourceSounds_;
};

}  // namespace core
