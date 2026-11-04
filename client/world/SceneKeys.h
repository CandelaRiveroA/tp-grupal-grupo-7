#pragma once

namespace world {


inline constexpr const char* kDrawModeSingle = "single";
inline constexpr const char* kDrawModeTile = "tile";
inline constexpr const char* kDrawModeHorizontalStrip = "horizontal_strip";

inline constexpr const char* kKeyTilesets = "tilesets";
inline constexpr const char* kKeyRooms = "rooms";

inline constexpr const char* kKeyImage = "image";
inline constexpr const char* kKeyCell = "cell";

inline constexpr const char* kKeyFloorColor = "floor_color";
inline constexpr const char* kKeyGrid = "grid";
inline constexpr const char* kKeyScale = "scale";
inline constexpr const char* kKeyElements = "elements";
inline constexpr const char* kKeySounds = "sounds";

// Poblaciones de la sala (OPCIONAL): qué puebla la fuente de estado.
inline constexpr const char* kKeySpawns = "spawns";
inline constexpr const char* kKeySpawnAnimals = "animals";
inline constexpr const char* kKeySpawnMinerals = "minerals";

inline constexpr const char* kKeyTileset = "tileset";
// La pieza del elemento: `region` (bloque contiguo de la hoja) o
// `piece` + `cells` (composición explícita, celda por celda).
inline constexpr const char* kKeyRegion = "region";
inline constexpr const char* kKeyPiece = "piece";
inline constexpr const char* kKeyCells = "cells";
inline constexpr const char* kKeyAt = "at";
inline constexpr const char* kKeySpan = "span";
inline constexpr const char* kKeyDraw = "draw";

inline constexpr const char* kKeyMode = "mode";
inline constexpr const char* kKeyScaleX = "scale_x";
inline constexpr const char* kKeyScaleY = "scale_y";

}
