#pragma once

#include <QPixmap>

#include "CloudRects.h"

class QPainter;

// Fondo completo de la pantalla de inicio
class TitleBackdrop {
public:
    TitleBackdrop();
    void paint(QPainter& painter) const;

    // Avanza la deriva de las nubes un intervalo en segundos.
    void step(float seconds);

private:
    QPixmap sky_;

    QPixmap clouds_[CLOUD_ZONE_COUNT];
    float cloudX_[CLOUD_ZONE_COUNT];
    int cloudY_[CLOUD_ZONE_COUNT];
    float cloudSpeed_[CLOUD_ZONE_COUNT];
};
