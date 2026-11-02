#pragma once

#include <QObject>
#include <QPixmap>
#include <QSize>

#include <cstdint>

#include "qt/util/CharacterLayer.h"

// Fuente de las capas del personaje: cuántas variantes hay por capa y el pixmap
// de cada una. Quien implementa decide cómo las arma; quien la usa las apila
// (todas comparten el mismo lienzo, sin offsets).
class CharacterLayerSource : public QObject {
	Q_OBJECT

public:
	explicit CharacterLayerSource(QObject* parent = nullptr);
	~CharacterLayerSource() override;

	virtual QSize canvasSize() const = 0;
	virtual std::uint8_t count(CharacterLayer layer) const = 0;
	// Lienzo completo de la capa. Las capas de color (HairColor) entregan un
	// color plano que el preview usa como tinte, no un sprite.
	virtual QPixmap pixmap(CharacterLayer layer, std::uint8_t index) const = 0;
};
