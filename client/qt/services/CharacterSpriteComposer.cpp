#include "CharacterSpriteComposer.h"

#include <QColor>
#include <QPainter>
#include <algorithm>
#include <map>

#include "qt/theme/CharacterSheetRects.h"
#include "world/CharacterAppearance.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

CharacterSpriteComposer::CharacterSpriteComposer(const world::CharacterCatalog& characters,
                                       const world::CharacterPalettes& palettes, QObject* parent):
        CharacterLayerSource(parent),
        sheets_(),
        skins_(sheets_.skins()),
        palettes_(palettes),
        bodyRect_(),
        armsRect_(),
        sleeve_(),
        builders_(),
        layers_() {
    readPose(characters);
    readSleeve();
    build();
}

QSize CharacterSpriteComposer::canvasSize() const { return QSize(SHEET_CANVAS_W, SHEET_CANVAS_H); }

std::uint8_t CharacterSpriteComposer::count(CharacterLayer layer) const {
    const auto found = layers_.find(layer);
    if (found == layers_.end()) {
        return 0;
    }
    return static_cast<std::uint8_t>(found->second.size());
}

QPixmap CharacterSpriteComposer::pixmap(CharacterLayer layer, std::uint8_t index) const {
    const auto found = layers_.find(layer);
    if (found == layers_.end() || index >= found->second.size()) {
        return QPixmap::fromImage(blankCanvas());
    }
    return found->second[index];
}

// Cuerpo y brazos del cuadro frente / idle, tal como los dibuja SDL.
void CharacterSpriteComposer::readPose(const world::CharacterCatalog& characters) {
    const world::CreatureKind kind = world::CreatureKind::Player;
    const int cellW = characters.sheetCellWidth(kind);
    const int cellH = characters.sheetCellHeight(kind);
    const world::DirectionAppearance& front =
            characters.dataFor(kind).directions[static_cast<int>(world::Direction::Front)];
    const world::ActionData& idle = front.actions.at(world::ActionKind::Idle);

    const std::map<world::Layer, QRect*> targets = {
            {world::Layer::Body, &bodyRect_},
            {world::Layer::Arms, &armsRect_},
    };
    for (const world::LayerFrame& layer: idle.frames.front()) {
        const auto target = targets.find(layer.layer);
        if (target != targets.end()) {
            *target->second = QRect(layer.frame.col * cellW, layer.frame.row * cellH,
                                    layer.cols * cellW, layer.rows * cellH);
        }
    }
    if (bodyRect_.isEmpty()) {
        throw world::ConfigError(world::ConfigErrorKind::MissingStructure,
                                 "characters.player.front.idle");
    }
}

// Los brazos se anclan abajo del cuerpo, igual que en SDL.
QPoint CharacterSpriteComposer::armsOrigin() const {
    return QPoint(SHEET_BODY_X, SHEET_BODY_Y + bodyRect_.height() - armsRect_.height());
}

void CharacterSpriteComposer::readSleeve() {
    if (armsRect_.isEmpty()) {
        return;
    }
    const std::map<QRgb, int> tones = {
            {SHEET_SLEEVE_DARK_RGB, 0},
            {SHEET_SLEEVE_MID_RGB, 1},
            {SHEET_SLEEVE_LIGHT_RGB, 2},
    };
    const QImage arms = sheets_.base().copy(armsRect_);
    for (int y = 0; y < arms.height(); ++y) {
        for (int x = 0; x < arms.width(); ++x) {
            const QRgb pixel = arms.pixel(x, y);
            const auto tone = tones.find(pixel & 0x00FFFFFF);
            if (qAlpha(pixel) == 255 && tone != tones.end()) {
                sleeve_.push_back({armsOrigin() + QPoint(x, y), tone->second});
            }
        }
    }
}

void CharacterSpriteComposer::build() {
    builders_ = {
            {CharacterLayer::Skin, &CharacterSpriteComposer::buildSkin},
            {CharacterLayer::Pants, &CharacterSpriteComposer::buildPants},
            {CharacterLayer::PantsColor, &CharacterSpriteComposer::buildPantsColor},
            {CharacterLayer::Shirt, &CharacterSpriteComposer::buildShirt},
            {CharacterLayer::Eyes, &CharacterSpriteComposer::buildEyes},
            {CharacterLayer::Accessory, &CharacterSpriteComposer::buildAccessory},
            {CharacterLayer::HairColor, &CharacterSpriteComposer::buildHairColor},
            {CharacterLayer::Hair, &CharacterSpriteComposer::buildHair},
    };
    for (const auto& entry: builders_) {
        (this->*entry.second)();
    }
}

QImage CharacterSpriteComposer::blankCanvas() const {
    QImage canvas(SHEET_CANVAS_W, SHEET_CANVAS_H, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    return canvas;
}

QPixmap CharacterSpriteComposer::placed(const QImage& piece, const QPoint& at) const {
    QImage canvas = blankCanvas();
    QPainter painter(&canvas);
    painter.drawImage(at, piece);
    painter.end();
    return QPixmap::fromImage(canvas);
}

void CharacterSpriteComposer::buildSkin() {
    QImage canvas = blankCanvas();
    QPainter painter(&canvas);
    painter.drawImage(QPoint(SHEET_BODY_X, SHEET_BODY_Y), sheets_.base().copy(bodyRect_));
    if (!armsRect_.isEmpty()) {
        painter.drawImage(armsOrigin(), sheets_.base().copy(armsRect_));
    }
    painter.end();

    for (int row = 0; row < skins_.count(); ++row) {
        layers_[CharacterLayer::Skin].push_back(QPixmap::fromImage(skins_.recolor(canvas, row)));
    }
}

// Se puede teñir si casi todos sus pixeles son grises.
bool CharacterSpriteComposer::isDyeable(const QImage& cell) const {
    int opaque = 0;
    int grey = 0;
    for (int y = 0; y < cell.height(); ++y) {
        for (int x = 0; x < cell.width(); ++x) {
            const QRgb pixel = cell.pixel(x, y);
            if (qAlpha(pixel) == 0) {
                continue;
            }
            ++opaque;
            const int spread = std::max({qRed(pixel), qGreen(pixel), qBlue(pixel)}) -
                               std::min({qRed(pixel), qGreen(pixel), qBlue(pixel)});
            grey += spread < SHEET_PANTS_GREY_SPREAD ? 1 : 0;
        }
    }
    return opaque > 0 && grey * 100 >= opaque * SHEET_PANTS_DYEABLE_PERCENT;
}

void CharacterSpriteComposer::buildPants() {
    for (int i = 0; i < SHEET_PANTS_CELLS; ++i) {
        const QRect cell(i * SHEET_PANTS_STRIDE_X, 0, SHEET_PANTS_W, SHEET_PANTS_H);
        const QImage image = sheets_.pants().copy(cell);
        if (!isDyeable(image)) {
            continue;
        }
        layers_[CharacterLayer::Pants].push_back(placed(image, QPoint(SHEET_BODY_X, SHEET_BODY_Y)));
    }
}

void CharacterSpriteComposer::buildPantsColor() {
    for (const std::array<int, 3>& rgb: palettes_.pants) {
        QImage flat = blankCanvas();
        flat.fill(QColor(rgb[0], rgb[1], rgb[2]));
        layers_[CharacterLayer::PantsColor].push_back(QPixmap::fromImage(flat));
    }
}

// Torso de 8x8 mas las mangas, con tres tonos tomados de la camisa.
void CharacterSpriteComposer::buildShirt() {
    for (int i = 0; i < SHEET_SHIRT_COUNT; ++i) {
        const int originX = (i % SHEET_SHIRT_COLUMNS) * SHEET_SHIRT_SIZE;
        const int originY = (i / SHEET_SHIRT_COLUMNS) * SHEET_SHIRT_STRIDE_Y;
        const QRect cell(originX, originY, SHEET_SHIRT_SIZE, SHEET_SHIRT_SIZE);

        QImage canvas = blankCanvas();
        QPainter painter(&canvas);
        painter.drawImage(QPoint(SHEET_BODY_X + SHEET_SHIRT_X, SHEET_BODY_Y + SHEET_SHIRT_Y),
                          sheets_.shirts().copy(cell));
        painter.end();

        const QRgb tones[] = {
                sheets_.shirts().pixel(originX + SHEET_SLEEVE_SAMPLE_X,
                                       originY + SHEET_SLEEVE_DARK_Y),
                sheets_.shirts().pixel(originX + SHEET_SLEEVE_SAMPLE_X,
                                       originY + SHEET_SLEEVE_MID_Y),
                sheets_.shirts().pixel(originX + SHEET_SLEEVE_SAMPLE_X,
                                       originY + SHEET_SLEEVE_LIGHT_Y),
        };
        for (const SleevePixel& pixel: sleeve_) {
            if (qAlpha(tones[pixel.tone]) == 255) {
                canvas.setPixel(pixel.at, tones[pixel.tone]);
            }
        }
        layers_[CharacterLayer::Shirt].push_back(QPixmap::fromImage(canvas));
    }
}

// Solo repinta las pupilas que ya trae el cuerpo.
void CharacterSpriteComposer::buildEyes() {
    const QImage body = sheets_.base().copy(bodyRect_);
    for (const std::array<int, 3>& rgb: palettes_.eyes) {
        const QColor color(rgb[0], rgb[1], rgb[2]);
        QImage eyes(body.size(), QImage::Format_ARGB32);
        eyes.fill(Qt::transparent);
        for (int y = 0; y < body.height(); ++y) {
            for (int x = 0; x < body.width(); ++x) {
                const QRgb pixel = body.pixel(x, y);
                if (qAlpha(pixel) == 255 && (pixel & 0x00FFFFFF) == SHEET_PUPIL_RGB) {
                    eyes.setPixelColor(x, y, color);
                }
            }
        }
        layers_[CharacterLayer::Eyes].push_back(placed(eyes, QPoint(SHEET_BODY_X, SHEET_BODY_Y)));
    }
}

void CharacterSpriteComposer::buildAccessory() {
    layers_[CharacterLayer::Accessory].push_back(QPixmap::fromImage(blankCanvas()));
    for (int k = 0; k < SHEET_ACC_COUNT - 1; ++k) {
        const QRect cell((k % SHEET_ACC_COLUMNS) * SHEET_ACC_SIZE,
                         (k / SHEET_ACC_COLUMNS) * SHEET_ACC_STRIDE_Y, SHEET_ACC_SIZE,
                         SHEET_ACC_SIZE);
        layers_[CharacterLayer::Accessory].push_back(
                placed(sheets_.accessories().copy(cell),
                       QPoint(SHEET_BODY_X, SHEET_BODY_Y + SHEET_ACC_OFFSET_Y)));
    }
}

void CharacterSpriteComposer::buildHairColor() {
    for (const std::array<int, 3>& rgb: palettes_.hair) {
        QImage flat = blankCanvas();
        flat.fill(QColor(rgb[0], rgb[1], rgb[2]));
        layers_[CharacterLayer::HairColor].push_back(QPixmap::fromImage(flat));
    }
}

void CharacterSpriteComposer::buildHair() {
    for (int i = 0; i < SHEET_HAIR_COUNT; ++i) {
        const QRect cell((i % SHEET_HAIR_COLUMNS) * SHEET_HAIR_W,
                         (i / SHEET_HAIR_COLUMNS) * SHEET_HAIR_STRIDE_Y, SHEET_HAIR_W,
                         SHEET_HAIR_H);
        layers_[CharacterLayer::Hair].push_back(
                placed(sheets_.hairstyles().copy(cell),
                       QPoint(SHEET_BODY_X, SHEET_BODY_Y + SHEET_HAIR_OFFSET_Y)));
    }
}
