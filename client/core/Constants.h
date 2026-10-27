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

inline constexpr const char* kServerHost = "127.0.0.1";
inline constexpr const char* kServerPort = "8080";

std::string assetPath(const std::string& file);

std::string soundPath(const std::string& file);

}  // namespace core
