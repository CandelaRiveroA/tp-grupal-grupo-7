#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/PixelBox.h"
#include "world/ScenePixelMapper.h"
#include "world/SliderLayout.h"
#include "world/SoundCategory.h"
#include "world/SpriteVisual.h"

namespace world {

// La pestañita sobre el borde superior izquierdo del panel.
struct SettingsTab {
    std::string sheet;
    PixelSize cell;
    SheetCell at;
    PixelSize size;
    // Corrimiento en x desde el borde izquierdo y cuánto se mete en el
    // panel por abajo.
    PixelSize offset;
};

// Los dos íconos del volumen (el normal y el del volumen 0).
struct SettingsIcon {
    std::string image;
    std::string mutedImage;
    PixelSize cell;
    PixelSize size;
};

// El bloque del porcentaje: dígitos más el signo.
struct SettingsPercent {
    int width = 0;
    std::string sign;
    int signWidth = 0;
    int digitSize = 0;
    int digitStep = 0;
};

// Una fila de audio tal como la declara el TOML.
struct VolumeRow {
    SoundCategory category = SoundCategory::Sfx;
    std::string label;
    int defaultPercent = 0;
};

// Dónde cae cada parte de una fila de volumen.
struct VolumeRowBoxes {
    SoundCategory category = SoundCategory::Sfx;
    PixelBox label;
    // El trayecto del deslizador: lo que se toca y por donde corre la
    // perilla (SliderLayout).
    PixelBox track;
    PixelBox icon;
    PixelBox percent;
};

// Dónde cae cada parte del popup dentro de un panel.
struct SettingsBoxes {
    PixelBox tab;
    PixelBox title;
    PixelBox multiplayerTitle;
    PixelBox qrLabel;
    // Espacio reservado para el código de la partida que mostrará el servidor.
    PixelBox qrArea;
    PixelBox qrButton;
    PixelBox audioTitle;
    std::vector<VolumeRowBoxes> volume;
    PixelBox leaveLabel;
    PixelBox leaveButton;
};

// Los dígitos del porcentaje y el signo, ya ubicados.
struct PercentBoxes {
    std::vector<PixelBox> digits;
    PixelBox sign;
};

// Datos y geometría del popup de configuración (sin SDL): carga
// ui/settings.toml en su constructor (lanza ConfigError) y calcula dónde
// cae cada cosa dentro del panel. Quien dibuja (sdl::SettingsPainter)
// solo pinta lo que esto calcula; quien maneja los clicks usa los mismos
// boxes, así lo que se toca es lo que se ve.
struct SettingsLayout {
    GridSpan grid{0, 0};
    Color textColor{0, 0, 0, 255};
    std::string title;
    int titleSize = 0;
    int subtitleSize = 0;
    int labelSize = 0;
    PixelSize padding;
    int gap = 0;
    int columnGap = 0;
    // Aire extra bajo el título principal (px).
    int titleGap = 0;
    // Aire extra antes de la sección de audio (px).
    int sectionGap = 0;
    int titleHeight = 0;
    int subtitleHeight = 0;
    int rowHeight = 0;
    int labelWidth = 0;

    SettingsTab tab;
    std::string multiplayerTitle;
    std::string qrLabel;
    // Ancho del espacio reservado para el código; el botón va pegado a él.
    int qrAreaWidth = 0;
    PixelSize qrButtonSize;
    // Corrimiento fino del botón respecto del centro de su fila (px).
    PixelSize qrButtonOffset;
    std::string audioTitle;

    // Fila final: la pregunta de abandonar y el botón de cerrar a su lado.
    std::string leaveLabel;
    int leaveLabelWidth = 0;
    PixelSize leaveButtonSize;
    PixelSize leaveButtonOffset;

    SliderLayout slider;
    std::string knobImage;
    SettingsIcon icon;
    SettingsPercent percent;

    // Una fila por categoría, en el orden en que se dibujan.
    std::vector<VolumeRow> volumeRows;

    explicit SettingsLayout(const std::string& path);

    // El volumen inicial de cada categoría declarada.
    std::map<SoundCategory, int> defaultVolumes() const;

    SettingsBoxes boxes(const PixelBox& panel) const;

    // Los dígitos de un porcentaje y su signo dentro del bloque de la
    // fila (alineados a la derecha).
    PercentBoxes percentBoxes(const PixelBox& percentArea, int digitCount) const;

    bool onQrButton(const SettingsBoxes& boxes, const PixelPoint& point) const;
    bool onLeaveButton(const SettingsBoxes& boxes, const PixelPoint& point) const;

    // La categoría cuyo deslizador está bajo el punto, si hay alguna.
    std::optional<SoundCategory> sliderAt(const SettingsBoxes& boxes,
                                          const PixelPoint& point) const;

private:
    bool contains(const PixelBox& box, const PixelPoint& point) const;

    VolumeRowBoxes volumeRowBoxes(const VolumeRow& row, const PixelBox& band,
                                  int innerRight) const;
};

}  // namespace world
