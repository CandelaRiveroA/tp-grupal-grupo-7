#pragma once

#include <QObject>
#include <QPixmap>
#include <QSize>

#include <cstdint>

#include "qt/util/CharacterLayer.h"

// Fuente de las imágenes del personaje. Responde cuántas variantes hay por
// capa y entrega el pixmap de cada una; no compone ni dibuja.
// Todas las capas comparten el mismo lienzo, así que se apilan sin offsets.
class AssetCatalog : public QObject {
	Q_OBJECT

public:
	explicit AssetCatalog(QObject* parent = nullptr);
	~AssetCatalog() override;

	virtual QSize canvasSize() const = 0;
	virtual std::uint8_t count(CharacterLayer layer) const = 0;
	// Lienzo completo de la capa. Las capas de color (HairColor) entregan un
	// color plano que el preview usa como tinte, no un sprite.
	virtual QPixmap pixmap(CharacterLayer layer, std::uint8_t index) const = 0;
};
