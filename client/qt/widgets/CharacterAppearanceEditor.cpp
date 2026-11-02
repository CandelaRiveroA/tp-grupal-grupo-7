#include "CharacterAppearanceEditor.h"

#include <QVBoxLayout>

#include "qt/services/CharacterLayerSource.h"
#include "qt/theme/CharacterRects.h"
#include "qt/theme/CharacterSprites.h"
#include "qt/util/IndexCycle.h"
#include "qt/widgets/CharacterPreview.h"
#include "qt/widgets/CycleRow.h"
#include "qt/widgets/SliderRow.h"

CharacterAppearanceEditor::CharacterAppearanceEditor(const CharacterLayerSource* source,
                                                     const CharacterSprites& sprites,
                                                     QObject* parent):
        QObject(parent), source_(source), sprites_(sprites) {}

QWidget* CharacterAppearanceEditor::previewColumn(QWidget* parent) {
    auto* column = new QWidget(parent);
    column->setObjectName("previewColumn");
    column->setFixedWidth(CHAR_PREVIEW_COL_W);
    auto* layout = new QVBoxLayout(column);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(CHAR_ROW_GAP);

    preview_ = new CharacterPreview(source_, column);
    preview_->setFixedSize(CHAR_PREVIEW_W, CHAR_PREVIEW_H);
    preview_->setBackground(sprites_.portraitBackground());
    layout->addWidget(preview_, 0, Qt::AlignHCenter);

    struct Row {
        CharacterLayer layer;
        const char* name;
    };
    const Row rows[] = {
            {CharacterLayer::Skin, "Piel"},           {CharacterLayer::Hair, "Pelo"},
            {CharacterLayer::Shirt, "Camisa"},        {CharacterLayer::Pants, "Pantalón"},
            {CharacterLayer::Accessory, "Accesorio"},
    };
    for (const Row& row: rows) {
        layout->addWidget(makeRow(row.layer, row.name, column));
    }
    return column;
}

QWidget* CharacterAppearanceEditor::sliderColumn(QWidget* parent) {
    auto* column = new QWidget(parent);
    auto* layout = new QVBoxLayout(column);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    layout->addWidget(makeSlider(CharacterLayer::Eyes, "Color de ojos", "eyesSlider", column));
    layout->addWidget(
            makeSlider(CharacterLayer::HairColor, "Color de pelo", "hairColorSlider", column));
    layout->addWidget(makeSlider(CharacterLayer::PantsColor, "Color de pantalón",
                                 "pantsColorSlider", column));
    return column;
}

const CharacterAppearance& CharacterAppearanceEditor::appearance() const { return appearance_; }

void CharacterAppearanceEditor::setAppearance(const CharacterAppearance& appearance) {
    appearance_ = appearance;
    applyAppearance();
}

QWidget* CharacterAppearanceEditor::makeRow(CharacterLayer layer, const QString& name,
                                            QWidget* parent) {
    auto* row = new CycleRow(name, sprites_.leftArrow(), sprites_.leftArrowHover(),
                             sprites_.rightArrow(), sprites_.rightArrowHover(), parent);
    connect(row, &CycleRow::stepRequested, this,
            [this, layer](int direction) { cycleLayer(layer, direction); });
    cycleRows_[layer] = row;
    return row;
}

QWidget* CharacterAppearanceEditor::makeSlider(CharacterLayer layer, const QString& name,
                                               const QString& objectName, QWidget* parent) {
    const int total = source_->count(layer);
    auto* row = new SliderRow(name, objectName, total > 0 ? total - 1 : 0, parent);
    connect(row, &SliderRow::valueChanged, this,
            [this, layer](int value) { setLayer(layer, static_cast<std::uint8_t>(value)); });
    sliderRows_[layer] = row;
    return row;
}

void CharacterAppearanceEditor::cycleLayer(CharacterLayer layer, int direction) {
    const std::uint8_t total = source_->count(layer);
    if (total == 0) {
        return;
    }

    const IndexCycle cycle(total);
    const std::uint8_t current = layers_.value(appearance_, layer);
    const std::uint8_t next = direction > 0 ? cycle.next(current) : cycle.previous(current);
    setLayer(layer, next);
}

void CharacterAppearanceEditor::setLayer(CharacterLayer layer, std::uint8_t index) {
    const std::uint8_t total = source_->count(layer);
    if (total == 0) {
        return;
    }
    if (index >= total) {
        index = static_cast<std::uint8_t>(total - 1);
    }

    layers_.set(appearance_, layer, index);
    applyAppearance();
}

void CharacterAppearanceEditor::applyAppearance() {
    preview_->setAppearance(appearance_);

    for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count); ++layer) {
        const auto current = static_cast<CharacterLayer>(layer);
        const std::uint8_t value = layers_.value(appearance_, current);

        const auto cycle = cycleRows_.find(current);
        if (cycle != cycleRows_.end()) {
            cycle->second->setValue(QString::number(value));
        }

        const auto slider = sliderRows_.find(current);
        if (slider != sliderRows_.end()) {
            slider->second->setValue(value);
        }
    }
    emit appearanceChanged();
}