#pragma once

#include <QImage>
#include <QRgb>

class SkinRecolorer {
public:
    explicit SkinRecolorer(const QImage& skins);
    int count() const;
    QImage recolor(const QImage& image, int row) const;

private:
    QImage skins_;
};
