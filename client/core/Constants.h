#pragma once

#include <cstddef>
#include <string>

#include "common/common_constants.h"

namespace core {


constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 700;
constexpr const char* kWindowTitle = "TP Final — Grupo 7";

constexpr int kTargetFps = 60;

constexpr int kIntroDurationMs = 2500;
constexpr int kIntroTicks = kIntroDurationMs * kTargetFps / 1000;
constexpr int kFirstIteration = 0;

constexpr int kFadeDurationMs = 1000;
constexpr int kFadeTicks = kFadeDurationMs * kTargetFps / 1000;

constexpr int kFadeOpaqueAlpha = 255;

// Lo que tarda la luz en ir de día pleno a noche plena (y a la inversa).
constexpr int kLightTransitionMs = 3000;
constexpr int kLightTransitionTicks = kLightTransitionMs * kTargetFps / 1000;

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
constexpr const char* kMarketRoom = "market";
constexpr const char* kNpcsFile = "npcs.toml";
constexpr const char* kMarketFile = "market.toml";

constexpr const char* kCharactersFile = "characters.toml";
constexpr const char* kEnemiesFile = "enemies.toml";
constexpr const char* kPalettesResource = ":/character/palettes.toml";

constexpr const char* kInventoryFile = "inventory.toml";

constexpr const char* kToolsOverlayFile = "tools_overlay.toml";

constexpr const char* kResourcesFile = "resources.toml";

constexpr const char* kObtainedBannerFile = "obtained_banner.toml";

constexpr const char* kDigitsFile = "ui/digits.toml";

constexpr const char* kCursorsFile = "ui/cursors.toml";

constexpr const char* kDialogsFile = "ui/dialogs.toml";

constexpr const char* kTextFile = "ui/text.toml";

constexpr const char* kChatFile = "ui/chatbox.toml";

constexpr const char* kHudOverlaysFile = "ui/hud.toml";

constexpr const char* kLightingFile = "ui/lighting.toml";

// ==== Chat ====

// Ticks del parpadeo del cursor en la línea de tipeo.
constexpr int kChatBlinkTicks = 20;

// Tope del texto tipeado: un paste largo encarece el wrap.
constexpr std::size_t kChatMaxLength = kMaxChatLength;

// Cuánto se queda el preview del chat cerrado después de un mensaje.
constexpr int kChatPreviewMs = 3000;
constexpr int kChatPreviewTicks = kChatPreviewMs * kTargetFps / 1000;

// Cantidad de slots del inventario ocupados por el mock (pico, pala,
// regadera, espada). Las teclas 1..kInventorySlots seleccionan cada uno.
constexpr int kInventorySlots = 4;

constexpr const char* kServerHost = "127.0.0.1";
constexpr const char* kServerPort = "8080";

std::string assetPath(const std::string& file);

std::string soundPath(const std::string& file);

}  // namespace core
