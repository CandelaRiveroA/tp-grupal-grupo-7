#pragma once

#include <QPixmap>
#include <QWidget>

#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "qt/util/CharacterLayers.h"

class AssetCatalog;
class QPaintEvent;

// Dibuja el personaje apilando las capas que entrega el AssetCatalog sobre un
// lienzo vertical.
class CharacterPreview : public QWidget {
	Q_OBJECT

public:
	explicit CharacterPreview(const AssetCatalog* catalog,
				  QWidget* parent = nullptr);

	void setAppearance(const CharacterAppearance& appearance);
	void setBackground(const QPixmap& background);

	QSize sizeHint() const override;

private:
	const AssetCatalog* catalog_;
	CharacterLayers layers_;
	CharacterAppearance appearance_;
	QPixmap background_;
	std::vector<QPixmap> drawables_;

	void refresh();
	QPixmap tintedHair(const QPixmap& hair, const QPixmap& tint) const;
	void paintEvent(QPaintEvent* event) override;
};
