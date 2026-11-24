#pragma once

#include <map>
#include <string>

#include "world/AnchorLayout.h"
#include "world/Color.h"
#include "world/PixelBox.h"
#include "world/WorldConstants.h"

namespace world {

// Datos puros (sin SDL) de los avisos en pantalla: hoja 9-slice del
// cartel, dónde se ancla, tipografía y el texto de cada id. La traducción
// vive en su constructor (§2): carga ui/notice.toml, valida y lanza
// ConfigError.
struct NoticeLayout {
    std::string sheetImage;
    int cellWidth = 0;
    int cellHeight = 0;
    // Esquinas fijas del marco (px de la hoja).
    int border = 0;

    Anchor anchor = Anchor::BottomCenter;
    int marginX = 0;
    int marginY = 0;
    int padX = 0;
    int padY = 0;

    int fontSize = 0;
    // Cara de fuente (FontCatalog::faces); "default" si el TOML no la dice.
    std::string fontFace = kDefaultFontFace;
    Color textColor{0, 0, 0, 255};
    int durationTicks = 0;

    // id del aviso -> texto que se muestra.
    std::map<std::string, std::string> texts;

    explicit NoticeLayout(const std::string& path);

    // nullptr si el id no tiene texto declarado.
    const std::string* textFor(const std::string& id) const;

    // Caja del cartel (ancla + tamaño del texto más padding) en la ventana.
    PixelBox boxFor(int textWidth, int windowWidth, int windowHeight) const;
};

}  // namespace world
