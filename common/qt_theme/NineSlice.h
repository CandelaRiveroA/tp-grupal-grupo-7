#pragma once

#include <QMargins>
#include <QPixmap>

// Arma un marco de cualquier tamaño desde un recorte 3x3 dividido por margenes:
// las esquinas se copian intactas y solo se estiran bordes y centro.
class NineSlice {
public:
    NineSlice(const QPixmap& frame, const QMargins& margins);

    // originalWidth/originalHeight: tamaño en píxeles antes de multiplicar por scale.
    QPixmap build(int originalWidth, int originalHeight, int scale) const;

private:
    QPixmap frame_;
    QMargins margins_;
};
