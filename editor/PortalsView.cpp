#include "PortalsView.h"

#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QVBoxLayout>

#include "Constants.h"

PortalsView::PortalsView(QWidget* parent)
	: QWidget(parent), rune_(":/editor/ui/portal_rune.png") {
	sizeCombo_ = new QComboBox(this);
	sizeCombo_->setObjectName("editorCombo");
	for (const SizeOption& opt : sizeOptions_) {
		sizeCombo_->addItem(opt.label);
	}

	list_ = new QListWidget(this);
	list_->setObjectName("portalList");
	list_->setViewMode(QListView::IconMode);
	list_->setIconSize(
		QSize(editor::kPaletteIconSize, editor::kPaletteIconSize));
	list_->setGridSize(editor::kPaletteGridSize);
	list_->setResizeMode(QListView::Adjust);
	list_->setMovement(QListView::Static);
	list_->setWordWrap(true);
	list_->setSpacing(editor::kPaletteItemSpacing);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kPaletteMargins);
	layout->addWidget(sizeCombo_);
	layout->addWidget(list_);

	populatePortals();

	connect(sizeCombo_, &QComboBox::currentIndexChanged, this, [this](int) {
		populatePortals();
	});
	connect(list_, &QListWidget::itemClicked, this,
		[this](QListWidgetItem* item) {
			if (!item) {
				return;
			}
			const SizeOption opt = currentSizeOption();
			const auto kind = static_cast<worldmodel::PortalKind>(
				item->data(Qt::UserRole).toInt());
			emit portalPicked(kind, opt.width, opt.height);
		});
}

void PortalsView::clearSelection() {
	list_->clearSelection();
}

PortalsView::SizeOption PortalsView::currentSizeOption() const {
	const int idx = sizeCombo_->currentIndex();
	const int count = static_cast<int>(sizeOptions_.size());
	return sizeOptions_[idx >= 0 && idx < count ? idx : 0];
}

QIcon PortalsView::portalPreviewIcon(int width, int height) const {
	QPixmap pix(editor::kPaletteIconSize, editor::kPaletteIconSize);
	pix.fill(Qt::transparent);
	QPainter p(&pix);
	const int blockW = width * editor::kPortalPreviewCellPx;
	const int blockH = height * editor::kPortalPreviewCellPx;
	const int x0 = (pix.width() - blockW) / 2;
	const int y0 = (pix.height() - blockH) / 2;
	for (int r = 0; r < height; ++r) {
		for (int c = 0; c < width; ++c) {
			p.drawPixmap(x0 + c * editor::kPortalPreviewCellPx,
				     y0 + r * editor::kPortalPreviewCellPx,
				     rune_);
		}
	}
	p.setBrush(Qt::NoBrush);
	p.drawRect(x0, y0, blockW, blockH);
	return QIcon(pix);
}

void PortalsView::populatePortals() {
	struct PortalEntry {
		worldmodel::PortalKind kind;
		const char* label;
	};
	constexpr std::array<PortalEntry, 7> kPortalEntries = {{
		{worldmodel::PortalKind::Link, "Portal link (libre)"},
		{worldmodel::PortalKind::TownEntrance,
		 "Entrada al pueblo (farm -> town)"},
		{worldmodel::PortalKind::FarmEntrance,
		 "Entrada a la granja (town -> farm)"},
		{worldmodel::PortalKind::DungeonEntrance,
		 "Entrada a mazmorra (farm -> dungeon_1)"},
		{worldmodel::PortalKind::StairsDown,
		 "Escaleras abajo (dungeon_n -> n+1)"},
		{worldmodel::PortalKind::DungeonExit,
		 "Salida de mazmorra (dungeon -> farm)"},
		{worldmodel::PortalKind::InteriorExit,
		 "Salida de interior (interior -> puerta)"},
	}};

	const SizeOption opt = currentSizeOption();
	const QIcon icon = portalPreviewIcon(opt.width, opt.height);

	list_->clear();
	for (const PortalEntry& entry : kPortalEntries) {
		QListWidgetItem* item = new QListWidgetItem(
			icon,
			QString("%1\n%2x%3")
				.arg(entry.label)
				.arg(opt.width)
				.arg(opt.height),
			list_);
		item->setData(Qt::UserRole, static_cast<int>(entry.kind));
	}
}
