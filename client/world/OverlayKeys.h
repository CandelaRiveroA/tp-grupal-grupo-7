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

// [decorations."..."] enter = { rows = [a, b], facing = "right",
// marker = "bed" }: la celda sale del marker (default "bed").
inline constexpr const char* kKeyOverlayEnterMarker = "marker";
inline constexpr const char* kDefaultEnterMarker = "bed";

// [markers.<kind>] npc = "<id>": el NPC que se ubica en ese marker.
inline constexpr const char* kKeyOverlayMarkers = "markers";
inline constexpr const char* kKeyOverlayNpc = "npc";

// [[maps.<id>.elements]]: decoraciones iniciales de un mapa.
inline constexpr const char* kKeyOverlayMaps = "maps";

}
