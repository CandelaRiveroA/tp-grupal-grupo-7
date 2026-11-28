#pragma once

namespace sdl {
namespace render {

// Constantes de presentación de la capa de render.
constexpr int kSpriteOverdrawMarginPixels = 128;
constexpr int kAxisCenterDivisor = 2;
constexpr int kScreenOrigin = 0;
constexpr double kFullSourceFraction = 1.0;

// Encuadre de la cámara. El mundo que entra en la ventana llena como mucho
// kCameraFitRatio del eje más restrictivo: deja aire y los NPCs no se ven
// gigantes. kCameraHudBandPixels es la banda inferior de la ventana que el
// mundo encuadrado deja libre (ahí vive el inventario). El zoom nunca baja
// de 1 (los mapas grandes scrollean) y el máximo es un tope de seguridad
// para mapas patológicamente chicos.
constexpr double kCameraFitRatio = 0.7;
constexpr int kCameraHudBandPixels = 94;
constexpr double kMinCameraZoom = 1.0;
constexpr double kMaxCameraZoom = 2.5;

}
}
