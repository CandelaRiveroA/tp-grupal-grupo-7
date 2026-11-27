#include "PortalsView.h"

#include <QComboBox>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <vector>

#include "Constants.h"
#include "PaletteListWidget.h"
#include "TilesetCache.h"
#include "world_model/PrefabCatalog.h"

PortalsView::PortalsView(const worldmodel::PrefabCatalog& catalog,
			 const TilesetCache& tilesets, QWidget* parent)
	: QWidget(parent),
	  catalog_(catalog),
	  tilesets_(tilesets),
	  rune_(":/editor/ui/portal_rune.png"),
	  sizeOptions_{
		  {"1x1 (1 celda)", 1, 1},
		  {"2x1 (horizontal)", 2, 1},
		  {"1x2 (vertical)", 1, 2},
		  {"2x2 (bloque)", 2, 2},
	  },
	  portalEntries_{
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
	  } {
	sizeCombo_ = new QComboBox(this);
	sizeCombo_->setObjectName("editorCombo");
	for (const SizeOption& opt : sizeOptions_) {
		sizeCombo_->addItem(opt.label);
	}

	list_ = new PaletteListWidget("portalList", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kPaletteMargins);
	layout->addWidget(sizeCombo_);
	layout->addWidget(list_, 1);

	populatePortals();

	connect(sizeCombo_, &QComboBox::currentIndexChanged, this, [this](int) {
		populatePortals();
	});
	connect(list_, &QListWidget::itemClicked, this,
		[this](QListWidgetItem* item) {
			if (!item) {
				return;
			}
			const auto kind = static_cast<worldmodel::PortalKind>(
				item->data(Qt::UserRole).toInt());
			if (const worldmodel::PrefabDef* prefab =
				    portalPrefabs_.prefabFor(kind, catalog_)) {
				emit portalPicked(kind, prefab->width,
						  prefab->height);
				return;
			}
			const SizeOption opt = currentSizeOption();
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
	const int blockW = width * editor::kPortalPreviewCellPx;
	const int blockH = height * editor::kPortalPreviewCellPx;
	QPixmap pix(blockW, blockH);
	pix.fill(Qt::transparent);
	QPainter p(&pix);
	for (int r = 0; r < height; ++r) {
		for (int c = 0; c < width; ++c) {
			p.drawPixmap(c * editor::kPortalPreviewCellPx,
				     r * editor::kPortalPreviewCellPx, rune_);
		}
	}
	p.setBrush(Qt::NoBrush);
	p.drawRect(0, 0, blockW - 1, blockH - 1);
	return tilesets_.centeredIcon(pix, editor::kPaletteIconSize);
}

void PortalsView::populatePortals() {
	const SizeOption opt = currentSizeOption();
	const QIcon defaultIcon = portalPreviewIcon(opt.width, opt.height);

	std::vector<PaletteListWidget::Card> cards;
	cards.reserve(portalEntries_.size());
	for (const PortalEntry& entry : portalEntries_) {
		const worldmodel::PrefabDef* prefab =
			portalPrefabs_.prefabFor(entry.kind, catalog_);
		const QIcon icon =
			prefab != nullptr
				? tilesets_.centeredIcon(
					  tilesets_.tilePixmap(prefab->tile,
							       prefab->width,
							       prefab->height),
					  editor::kPaletteIconSize)
				: defaultIcon;
		const int w = prefab != nullptr ? prefab->width : opt.width;
		const int h = prefab != nullptr ? prefab->height : opt.height;
		const QString label =
			QString("%1\n%2x%3").arg(entry.label).arg(w).arg(h);
		cards.push_back({icon, label, static_cast<int>(entry.kind)});
	}
	list_->setCards(cards);
}
