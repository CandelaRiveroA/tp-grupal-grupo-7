#include "FakeAssetCatalog.h"

#include <QPainter>
#include <QRect>

#include <cstddef>

FakeAssetCatalog::FakeAssetCatalog(QObject* parent)
	: AssetCatalog(parent)
	, layers_() {
	build();
}

QSize FakeAssetCatalog::canvasSize() const { return QSize(32, 48); }

std::uint8_t FakeAssetCatalog::count(CharacterLayer layer) const {
	return static_cast<std::uint8_t>(palette(layer).size());
}

QPixmap FakeAssetCatalog::pixmap(CharacterLayer layer,
				 std::uint8_t index) const {
	const auto images = layers_.find(layer);
	if (images == layers_.end() || images->second.empty()) {
		return QPixmap();
	}
	if (index >= images->second.size()) {
		return images->second.front();
	}
	return images->second[index];
}

void FakeAssetCatalog::build() {
	for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count);
	     ++layer) {
		buildLayer(static_cast<CharacterLayer>(layer));
	}
}

void FakeAssetCatalog::buildLayer(CharacterLayer layer) {
	const std::vector<QColor> colors = palette(layer);
	std::vector<QPixmap>& images = layers_[layer];
	images.reserve(colors.size());

	for (std::size_t index = 0; index < colors.size(); ++index) {
		images.push_back(
			render(layer, static_cast<std::uint8_t>(index), colors[index]));
	}
}

std::vector<QColor> FakeAssetCatalog::palette(CharacterLayer layer) const {
	switch (layer) {
	case CharacterLayer::Skin:
		return {QColor(0xF2, 0xD3, 0xB5), QColor(0xE8, 0xC0, 0x97),
			QColor(0xC9, 0x95, 0x6B), QColor(0x8D, 0x5F, 0x3F)};
	case CharacterLayer::Eyes:
		return {QColor(0x3B, 0x5B, 0x8C), QColor(0x5C, 0x3A, 0x1E),
			QColor(0x3F, 0x6B, 0x3A), QColor(0x44, 0x44, 0x44)};
	case CharacterLayer::Hair:
		// El color real lo pone el tinte de HairColor; acá sólo importa la
		// silueta, que cambia con el estilo.
		return std::vector<QColor>(6, QColor(0xC0, 0xC0, 0xC0));
	case CharacterLayer::HairColor:
		return {QColor(0x2B, 0x1B, 0x10), QColor(0x6B, 0x3F, 0x1F),
			QColor(0xD8, 0xB4, 0x5A), QColor(0xA8, 0x3A, 0x22),
			QColor(0x3C, 0x4C, 0x8C), QColor(0xDD, 0xDD, 0xDD)};
	case CharacterLayer::Shirt:
		return {QColor(0x3C, 0x6E, 0xA5), QColor(0xA5, 0x3C, 0x4E),
			QColor(0x4E, 0x8A, 0x3C), QColor(0x8A, 0x6E, 0x3C),
			QColor(0x6E, 0x3C, 0x8A), QColor(0x2F, 0x2F, 0x2F)};
	case CharacterLayer::Pants:
		return {QColor(0x33, 0x3F, 0x5C), QColor(0x5C, 0x3F, 0x33),
			QColor(0x3F, 0x5C, 0x4A), QColor(0x5C, 0x5C, 0x33),
			QColor(0x4A, 0x3F, 0x5C), QColor(0x70, 0x70, 0x70)};
	case CharacterLayer::Accessory:
		return {QColor(Qt::transparent), QColor(0x8C, 0x2F, 0x2F),
			QColor(0x33, 0x33, 0x33), QColor(0xE0, 0x7A, 0x2F)};
	case CharacterLayer::Count:
		break;
	}
	return {};
}

QPixmap FakeAssetCatalog::render(CharacterLayer layer, std::uint8_t index,
				 const QColor& color) const {
	QPixmap image(canvasSize());
	image.fill(Qt::transparent);

	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing, false);
	paintLayer(painter, layer, index, color);
	painter.end();
	return image;
}

void FakeAssetCatalog::paintLayer(QPainter& painter, CharacterLayer layer,
				  std::uint8_t index,
				  const QColor& color) const {
	switch (layer) {
	case CharacterLayer::Skin:
		painter.fillRect(QRect(10, 4, 12, 12), color);
		painter.fillRect(QRect(7, 19, 3, 10), color);
		painter.fillRect(QRect(22, 19, 3, 10), color);
		break;
	case CharacterLayer::Eyes:
		painter.fillRect(QRect(13, 10, 2, 2), color);
		painter.fillRect(QRect(18, 10, 2, 2), color);
		break;
	case CharacterLayer::Hair:
		painter.fillRect(QRect(9, 2, 14, 5), color);
		if (index == 1 || index == 3) {
			painter.fillRect(QRect(9, 6, 2, 8), color);
			painter.fillRect(QRect(21, 6, 2, 8), color);
		}
		if (index == 2) {
			painter.fillRect(QRect(13, 0, 6, 3), color);
		}
		if (index == 4) {
			painter.fillRect(QRect(14, 0, 4, 6), color);
		}
		if (index == 5) {
			painter.fillRect(QRect(8, 6, 3, 12), color);
		}
		break;
	case CharacterLayer::HairColor:
		painter.fillRect(
			QRect(0, 0, canvasSize().width(), canvasSize().height()),
			color);
		break;
	case CharacterLayer::Shirt:
		painter.fillRect(QRect(10, 18, 12, 12), color);
		painter.fillRect(QRect(7, 18, 3, 5), color);
		painter.fillRect(QRect(22, 18, 3, 5), color);
		break;
	case CharacterLayer::Pants:
		painter.fillRect(QRect(10, 30, 5, 15), color);
		painter.fillRect(QRect(17, 30, 5, 15), color);
		break;
	case CharacterLayer::Accessory:
		if (index == 1) {
			painter.fillRect(QRect(7, 1, 18, 3), color);
			painter.fillRect(QRect(11, 0, 10, 2), color);
		}
		if (index == 2) {
			painter.fillRect(QRect(12, 9, 4, 4), color);
			painter.fillRect(QRect(17, 9, 4, 4), color);
		}
		if (index == 3) {
			painter.fillRect(QRect(10, 17, 12, 4), color);
		}
		break;
	case CharacterLayer::Count:
		break;
	}
}
