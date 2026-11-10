#include "SkinRecolorer.h"

#include <algorithm>

namespace {
constexpr int kWitnessCount = 3;
constexpr QRgb kRgbMask = 0x00FFFFFF;
}  // namespace

SkinRecolorer::SkinRecolorer(const QImage& skins): skins_(skins) {}

int SkinRecolorer::count() const { return std::max(1, skins_.height()); }

QImage SkinRecolorer::recolor(const QImage& image, int row) const {
    QImage result = image;
    if (skins_.isNull() || skins_.width() < kWitnessCount) {
        return result;
    }
    const int line = std::clamp(row, 0, skins_.height() - 1);
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            const QRgb pixel = result.pixel(x, y);
            if (qAlpha(pixel) != 255) {
                continue;
            }
            for (int i = 0; i < kWitnessCount; ++i) {
                if ((pixel & kRgbMask) == (skins_.pixel(i, 0) & kRgbMask)) {
                    result.setPixel(x, y, skins_.pixel(i, line));
                    break;
                }
            }
        }
    }
    return result;
}
