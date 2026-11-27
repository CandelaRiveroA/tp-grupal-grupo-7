#pragma once

#include <QPixmap>
#include <QSize>

#include "CloudRects.h"

class QPainter;

// Fondo completo de la pantalla de inicio
class TitleBackdrop {
public:
    TitleBackdrop();
    void paint(QPainter& painter) const;

    // Reescala el cielo a `size` y hace el wrap de las nubes en su ancho.
    // Para ventanas redimensionables; el lobby (1280x720 fijo) no la llama.
    void resize(const QSize& size);

    // Avanza la deriva de las nubes un intervalo en segundos.
    void step(float seconds);

private:
    QPixmap skySource_;
    QPixmap sky_;
    int wrapWidth_;

    QPixmap clouds_[CLOUD_ZONE_COUNT];
    float cloudX_[CLOUD_ZONE_COUNT];
    int cloudY_[CLOUD_ZONE_COUNT];
    float cloudSpeed_[CLOUD_ZONE_COUNT];
};
