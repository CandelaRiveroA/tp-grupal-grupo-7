#include "CharacterPreview.h"

#include <QPainter>
#include <QPalette>

#include <algorithm>

#include "qt/services/AssetCatalog.h"

CharacterPreview::CharacterPreview(const AssetCatalog* catalog,
				   QWidget* parent)
	: QWidget(parent)
	, catalog_(catalog)
	, layers_()
	, appearance_()
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
	if (catalog_ == nullptr) {
		return QSize(192, 288);
	}
	return catalog_->canvasSize() * 6;
}

void CharacterPreview::refresh() {
	drawables_.clear();
	if (catalog_ == nullptr) {
		update();
		return;
	}

	const QPixmap tint = catalog_->pixmap(
		CharacterLayer::HairColor,
		layers_.value(appearance_, CharacterLayer::HairColor));

	for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count);
	     ++layer) {
		const auto current = static_cast<CharacterLayer>(layer);
		// El color de pelo no es un sprite: se aplica como tinte sobre el
		// pelo, así que no entra en la lista de capas dibujables.
		if (current == CharacterLayer::HairColor) {
			continue;
		}

		const QPixmap image = catalog_->pixmap(
			current, layers_.value(appearance_, current));
		if (current == CharacterLayer::Hair) {
			drawables_.push_back(tintedHair(image, tint));
		} else {
			drawables_.push_back(image);
		}
	}
	update();
}

QPixmap CharacterPreview::tintedHair(const QPixmap& hair,
				     const QPixmap& tint) const {
	if (hair.isNull() || tint.isNull()) {
		return hair;
	}

	QPixmap result(hair);
	QPainter painter(&result);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(result.rect(), tint);
	painter.end();
	return result;
}

void CharacterPreview::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	if (background_.isNull()) {
		painter.fillRect(rect(), palette().color(QPalette::Base));
	} else {
		painter.drawPixmap(rect(), background_);
	}

	if (catalog_ == nullptr) {
		return;
	}

	const QSize canvas = catalog_->canvasSize();
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
