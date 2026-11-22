#pragma once

namespace world {

// Claves de overlay.toml: los datos del CLIENTE que completan el mundo
// del editor. Los elementos extra y las hojas usan las claves de
// SceneKeys.h (misma sintaxis que scenes.toml).
inline constexpr const char* kKeyOverlayWorld = "world";
inline constexpr const char* kKeyOverlayScale = "scale";

// [decorations."tileset:col,row"]: la clave es el tile de origen de la
// decoración tal como lo guarda el editor.
inline constexpr const char* kKeyOverlayDecorations = "decorations";
inline constexpr const char* kKeyOverlayNightTile = "night_tile";

// [[maps.<id>.elements]]: decoraciones iniciales de un mapa.
inline constexpr const char* kKeyOverlayMaps = "maps";

}
