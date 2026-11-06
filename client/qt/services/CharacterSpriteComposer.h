#pragma once

#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <cstdint>
#include <map>
#include <vector>

#include "qt/services/CharacterLayerSource.h"
#include "qt/theme/CharacterSheets.h"
#include "qt/theme/SkinRecolorer.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterPalettes.h"

// Catalogo con las hojas reales de Stardew: arma al construirse todas las variantes
// en un lienzo de 32x48. Cuerpo y brazos salen de characters.toml, como en SDL.
class CharacterSpriteComposer: public CharacterLayerSource {
    Q_OBJECT

public:
    CharacterSpriteComposer(const world::CharacterCatalog& characters,
                       const world::CharacterPalettes& palettes, QObject* parent = nullptr);

    QSize canvasSize() const override;
    std::uint8_t count(CharacterLayer layer) const override;
    QPixmap pixmap(CharacterLayer layer, std::uint8_t index) const override;

private:
    using Builder = void (CharacterSpriteComposer::*)();

    // Pixel de manga en los brazos y su tono (0 oscuro, 1 medio, 2 claro).
    struct SleevePixel {
        QPoint at;
        int tone;
    };

    CharacterSheets sheets_;
    SkinRecolorer skins_;
    world::CharacterPalettes palettes_;
    QRect bodyRect_;
    QRect armsRect_;
    std::vector<SleevePixel> sleeve_;
    std::map<CharacterLayer, Builder> builders_;
    std::map<CharacterLayer, std::vector<QPixmap>> layers_;

    void readPose(const world::CharacterCatalog& characters);
    void readSleeve();
    void build();
    void buildSkin();
    void buildPants();
    void buildPantsColor();
    void buildShirt();
    void buildEyes();
    void buildAccessory();
    void buildHairColor();
    void buildHair();

    QImage blankCanvas() const;
    bool isDyeable(const QImage& cell) const;
    QPixmap placed(const QImage& piece, const QPoint& at) const;
    QPoint armsOrigin() const;
};
