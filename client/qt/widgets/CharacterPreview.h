#pragma once

#include <QPixmap>
#include <QWidget>

#include <map>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "qt/util/CharacterLayer.h"
#include "qt/util/CharacterLayers.h"

class CharacterLayerSource;
class QPaintEvent;

// Dibuja el personaje apilando las capas que entrega el CharacterLayerSource sobre un
// lienzo vertical.
class CharacterPreview : public QWidget {
	Q_OBJECT

public:
	explicit CharacterPreview(const CharacterLayerSource* source,
				  QWidget* parent = nullptr);

	void setAppearance(const CharacterAppearance& appearance);
	void setBackground(const QPixmap& background);

	QSize sizeHint() const override;

private:
	const CharacterLayerSource* source_;
	CharacterLayers layers_;
	CharacterAppearance appearance_;
	std::map<CharacterLayer, CharacterLayer> tints_;
	QPixmap background_;
	std::vector<QPixmap> drawables_;

	void refresh();
	QPixmap tinted(const QPixmap& sprite, const QPixmap& tint) const;
	void paintEvent(QPaintEvent* event) override;
};
