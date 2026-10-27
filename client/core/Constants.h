#pragma once

#include <string>

namespace core {


inline constexpr int kWindowWidth = 960;
inline constexpr int kWindowHeight = 640;
inline constexpr const char* kWindowTitle = "TP Final — Grupo 7";

inline constexpr int kTargetFps = 60;

inline constexpr int kIntroDurationMs = 2500;
inline constexpr int kIntroTicks = kIntroDurationMs * kTargetFps / 1000;

inline constexpr int kFadeDurationMs = 1000;
inline constexpr int kFadeTicks = kFadeDurationMs * kTargetFps / 1000;

inline constexpr int kFadeOpaqueAlpha = 255;

#ifndef TALLER_ASSET_DIR
#define TALLER_ASSET_DIR "assets"
#endif
inline const std::string kAssetsDir = TALLER_ASSET_DIR;

#ifndef TALLER_SOUNDS_DIR
#define TALLER_SOUNDS_DIR "sounds"
#endif
inline const std::string kSoundsDir = TALLER_SOUNDS_DIR;

inline constexpr const char* kScenesFile = "scenes.toml";
inline constexpr const char* kInitialRoom = "bedroom";

inline constexpr const char* kCharactersFile = "characters.toml";

inline constexpr const char* kInventoryFile = "inventory.toml";

inline constexpr const char* kToolsOverlayFile = "tools_overlay.toml";

inline constexpr const char* kResourcesFile = "resources.toml";

inline constexpr const char* kObtainedBannerFile = "obtained_banner.toml";

// Cantidad de slots del inventario ocupados por el mock (pico, pala,
// regadera, espada). Las teclas 1..kInventorySlots seleccionan cada uno.
inline constexpr int kInventorySlots = 4;

std::string assetPath(const std::string& file);

std::string soundPath(const std::string& file);

}  // namespace core
