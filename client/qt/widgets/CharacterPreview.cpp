#include "CharacterPreview.h"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <algorithm>
#include <map>

#include "qt/services/CharacterLayerSource.h"
#include "qt/theme/CharacterSheetRects.h"

CharacterPreview::CharacterPreview(const CharacterLayerSource* source,
				   QWidget* parent)
	: QWidget(parent)
	, source_(source)
	, layers_()
	, appearance_()
	, tints_({{CharacterLayer::Hair, CharacterLayer::HairColor},
		  {CharacterLayer::Pants, CharacterLayer::PantsColor}})
	, background_()
	, drawables_() {
	setObjectName("characterPreview");
	refresh();
}

void CharacterPreview::setAppearance(const CharacterAppearance& appearance) {
	appearance_ = appearance;
	refresh();
}

void CharacterPreview::setBackground(const QPixmap& background) {
	background_ = background;
	update();
}

QSize CharacterPreview::sizeHint() const {
	if (source_ == nullptr) {
		return QSize(SHEET_CANVAS_W, SHEET_CANVAS_H) * SHEET_PREVIEW_HINT_SCALE;
	}
	return source_->canvasSize() * SHEET_PREVIEW_HINT_SCALE;
}

void CharacterPreview::refresh() {
	drawables_.clear();
	if (source_ == nullptr) {
		update();
		return;
	}

	// Las capas de color aportan el tinte de otra capa; no se dibujan solas.
	std::map<CharacterLayer, bool> isTint;
	for (const auto& entry : tints_) {
		isTint[entry.second] = true;
	}

	for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count);
	     ++layer) {
		const auto current = static_cast<CharacterLayer>(layer);
		if (isTint.count(current) > 0) {
			continue;
		}

		const QPixmap image = source_->pixmap(
			current, layers_.value(appearance_, current));
		const auto tint = tints_.find(current);
		if (tint == tints_.end()) {
			drawables_.push_back(image);
			continue;
		}
		const CharacterLayer colorLayer = tint->second;
		drawables_.push_back(tinted(
			image, source_->pixmap(colorLayer,
						layers_.value(appearance_,
							      colorLayer))));
	}
	update();
}

// Multiplica cada pixel por el tinte: conserva el sombreado y el alfa.
QPixmap CharacterPreview::tinted(const QPixmap& sprite,
				  const QPixmap& tint) const {
	if (sprite.isNull() || tint.isNull()) {
		return sprite;
	}

	const QColor color = tint.toImage().pixelColor(0, 0);
	QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32);
	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			const QRgb pixel = image.pixel(x, y);
			if (qAlpha(pixel) == 0) {
				continue;
			}
			image.setPixel(x, y,
				       qRgba(qRed(pixel) * color.red() / 255,
					     qGreen(pixel) * color.green() / 255,
					     qBlue(pixel) * color.blue() / 255,
					     qAlpha(pixel)));
		}
	}
	return QPixmap::fromImage(image);
}

void CharacterPreview::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	// Sin background el preview queda transparente: uso la casilla del
	// contenedor en vez de pintar un relleno propio.
	if (!background_.isNull()) {
		painter.drawPixmap(rect(), background_);
	}

	if (source_ == nullptr) {
		return;
	}

	const QSize canvas = source_->canvasSize();
	if (canvas.isEmpty() || width() <= 0 || height() <= 0) {
		return;
	}

	const int scale =
		std::max(1, std::min(width() / canvas.width(),
				     height() / canvas.height()));
	const QSize target(canvas.width() * scale, canvas.height() * scale);
	const QPoint origin((width() - target.width()) / 2,
			    (height() - target.height()) / 2);

	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	for (const QPixmap& image : drawables_) {
		painter.drawPixmap(QRect(origin, target), image);
	}
}
