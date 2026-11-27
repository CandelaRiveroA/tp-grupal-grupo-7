#include "PaletteListWidget.h"

#include <QFontMetrics>
#include <QListView>
#include <QListWidgetItem>
#include <QRect>
#include <QSize>
#include <Qt>
#include <algorithm>

#include "Constants.h"

PaletteListWidget::PaletteListWidget(const QString& objectName, QWidget* parent)
	: QListWidget(parent) {
	setObjectName(objectName);
	setViewMode(QListView::IconMode);
	setMovement(QListView::Static);
	setResizeMode(QListView::Adjust);
	setWrapping(true);
	setWordWrap(true);
	setTextElideMode(Qt::ElideNone);
	setUniformItemSizes(true);
	setIconSize(QSize(editor::kPaletteIconSize, editor::kPaletteIconSize));
	setGridSize(QSize(editor::kPaletteCellWidth,
			  editor::kPaletteMinCellHeight));
	setSpacing(editor::kPaletteItemSpacing);
}

void PaletteListWidget::setCards(const std::vector<Card>& cards) {
	clear();

	const QFontMetrics metrics = fontMetrics();
	const int textWidth =
		editor::kPaletteCellWidth - editor::kPaletteTextInset;
	const int wrapFlags =
		Qt::AlignHCenter | Qt::TextWordWrap | Qt::TextWrapAnywhere;
	int maxLabelHeight = 0;

	for (const Card& card : cards) {
		auto* item = new QListWidgetItem(card.icon, card.label, this);
		item->setToolTip(card.label);
		item->setData(Qt::UserRole, card.userData);

		const int labelHeight =
			metrics.boundingRect(QRect(0, 0, textWidth, 0),
					     wrapFlags, card.label)
				.height();
		maxLabelHeight = std::max(maxLabelHeight, labelHeight);
	}

	// Reserva altura uniforme suficiente para el nombre completo sin recorte.
	const int cellHeight = std::max(
		editor::kPaletteMinCellHeight,
		editor::kPaletteIconSize + maxLabelHeight +
			editor::kPaletteCellPadding);
	setGridSize(QSize(editor::kPaletteCellWidth, cellHeight));
}
