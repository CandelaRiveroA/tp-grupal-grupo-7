#pragma once

#include "world/Color.h"
#include "world/PixelBox.h"

namespace world {

// Geometría pura (sin SDL) de un deslizador horizontal: una línea fina
// con una perilla que se corre de punta a punta del trayecto. Todo se
// calcula sobre la caja del trayecto (`track`), cuyo alto es el de la
// perilla; el valor es un porcentaje entero. Sirve para cualquier
// deslizador (hoy el volumen), no sabe qué valor controla.
struct SliderLayout {
    int lineThickness = 0;
    PixelSize knob;
    Color lineColor{0, 0, 0, 255};

    // La línea: centrada en vertical y entre los centros de la perilla
    // en 0% y en 100%.
    PixelBox lineBox(const PixelBox& track) const;

    // La perilla para ese porcentaje (se limita a 0-100).
    PixelBox knobBox(const PixelBox& track, int percent) const;

    // Inversa de knobBox: el porcentaje que deja el centro de la
    // perilla en la x pedida (limitado a 0-100).
    int percentAt(const PixelBox& track, int x) const;
};

}  // namespace world
