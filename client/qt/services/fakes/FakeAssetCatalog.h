#pragma once

#include <QColor>

#include <map>
#include <vector>

#include "qt/services/AssetCatalog.h"

class QPainter;

// Catálogo de prueba: genera cada capa como rectángulos de color plano, sin
// archivos ni estilos. El índice 0 es válido en todas las capas.
class FakeAssetCatalog : public AssetCatalog {
	Q_OBJECT

public:
	explicit FakeAssetCatalog(QObject* parent = nullptr);

	QSize canvasSize() const override;
	std::uint8_t count(CharacterLayer layer) const override;
	QPixmap pixmap(CharacterLayer layer, std::uint8_t index) const override;

private:
	std::map<CharacterLayer, std::vector<QPixmap>> layers_;

	void build();
	void buildLayer(CharacterLayer layer);
	std::vector<QColor> palette(CharacterLayer layer) const;
	QPixmap render(CharacterLayer layer, std::uint8_t index,
		       const QColor& color) const;
	void paintLayer(QPainter& painter, CharacterLayer layer,
			std::uint8_t index, const QColor& color) const;
};
