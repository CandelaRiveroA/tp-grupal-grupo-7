#pragma once

#include <QObject>
#include <QString>
#include <cstdint>
#include <map>

#include "common/model/draft/CharacterAppearance.h"
#include "qt/util/CharacterLayers.h"

class CharacterLayerSource;
class CharacterPreview;
class CharacterSprites;
class CycleRow;
class QWidget;
class SliderRow;

// Edita la apariencia del personaje: retrato, filas de ciclo y sliders.
// La pantalla decide dónde va cada columna.
class CharacterAppearanceEditor: public QObject {
    Q_OBJECT

public:
    CharacterAppearanceEditor(const CharacterLayerSource* source, const CharacterSprites& sprites,
                              QObject* parent = nullptr);

    // Columnas de controles; la pantalla las coloca en su ventana.
    QWidget* previewColumn(QWidget* parent);
    QWidget* sliderColumn(QWidget* parent);

    const CharacterAppearance& appearance() const;
    void setAppearance(const CharacterAppearance& appearance);

signals:
    void appearanceChanged();

private:
    QWidget* makeRow(CharacterLayer layer, const QString& name, QWidget* parent);
    QWidget* makeSlider(CharacterLayer layer, const QString& name, const QString& objectName,
                        QWidget* parent);
    void cycleLayer(CharacterLayer layer, int direction);
    void setLayer(CharacterLayer layer, std::uint8_t index);
    void applyAppearance();

    const CharacterLayerSource* source_;
    const CharacterSprites& sprites_;
    CharacterLayers layers_;
    CharacterPreview* preview_ = nullptr;
    std::map<CharacterLayer, CycleRow*> cycleRows_;
    std::map<CharacterLayer, SliderRow*> sliderRows_;
    CharacterAppearance appearance_;
};