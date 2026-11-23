#pragma once

#include <string>

#include "world/Color.h"
#include "world/ScenePixelMapper.h"
#include "world/WorldConstants.h"

namespace world {

// Datos y geometría PURA (sin SDL) del nombre bajo los pies: tipografía,
// separación y color según la hora. La traducción vive en su constructor
// (§2): carga la sección [nameplate] de ui/text.toml, valida y lanza
// ConfigError. Dónde cae la caja y qué color toca lo decide esta clase;
// sdl::NameplatePainter solo pinta.
class NameplateLayout {
public:
    explicit NameplateLayout(const std::string& path);

    // Caja del texto (px de mundo), centrada en X bajo el borde inferior
    // (los pies) del rectángulo dibujado de la entidad.
    PixelRect boxUnder(const PixelRect& drawn, int textWidth) const;

    // night_color desde night_from_hour; day_color antes.
    const Color& colorAt(int hour) const;

    const std::string& fontFace() const;
    int fontSize() const;
    bool bold() const;

private:
    std::string fontFace_ = kDefaultFontFace;
    int fontSize_ = 0;
    bool bold_ = false;
    int gap_ = 0;
    int boxHeight_ = 0;
    Color dayColor_{0, 0, 0, 255};
    Color nightColor_{255, 255, 255, 255};
    int nightFromHour_ = 0;
};

}  // namespace world
