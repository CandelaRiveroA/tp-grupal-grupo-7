#pragma once

#include <string>

namespace core {


constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 640;
constexpr const char* kWindowTitle = "TP Final — Grupo 7";

constexpr int kTargetFps = 60;

constexpr int kIntroDurationMs = 2500;
constexpr int kIntroTicks = kIntroDurationMs * kTargetFps / 1000;
constexpr int kFirstIteration = 0;

constexpr int kFadeDurationMs = 1000;
constexpr int kFadeTicks = kFadeDurationMs * kTargetFps / 1000;

constexpr int kFadeOpaqueAlpha = 255;

#ifndef TALLER_ASSET_DIR
#define TALLER_ASSET_DIR "assets"
#endif
constexpr const char* kAssetsDir = TALLER_ASSET_DIR;

#ifndef TALLER_SOUNDS_DIR
#define TALLER_SOUNDS_DIR "sounds"
#endif
constexpr const char* kSoundsDir = TALLER_SOUNDS_DIR;

constexpr const char* kScenesFile = "scenes.toml";
constexpr const char* kInitialRoom = "bedroom";

constexpr const char* kCharactersFile = "characters.toml";

constexpr const char* kInventoryFile = "inventory.toml";

constexpr const char* kToolsOverlayFile = "tools_overlay.toml";

constexpr const char* kResourcesFile = "resources.toml";

constexpr const char* kObtainedBannerFile = "obtained_banner.toml";

constexpr const char* kDigitsFile = "ui/digits.toml";

constexpr const char* kCursorsFile = "ui/cursors.toml";

constexpr const char* kDialogsFile = "ui/dialogs.toml";

// Cantidad de slots del inventario ocupados por el mock (pico, pala,
// regadera, espada). Las teclas 1..kInventorySlots seleccionan cada uno.
constexpr int kInventorySlots = 4;

constexpr const char* kServerHost = "127.0.0.1";
constexpr const char* kServerPort = "8080";

std::string assetPath(const std::string& file);

std::string soundPath(const std::string& file);

}  // namespace core
