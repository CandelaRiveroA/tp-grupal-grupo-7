#include "PaletteView.h"

#include <QComboBox>
#include <QListWidgetItem>
#include <QMetaType>
#include <QPixmap>
#include <QString>
#include <QVariant>
#include <QVBoxLayout>
#include <vector>

#include "Constants.h"
#include "MarkerSprites.h"
#include "PaletteListWidget.h"
#include "TilesetCache.h"
#include "world_model/PrefabCatalog.h"

PaletteView::PaletteView(const worldmodel::PrefabCatalog& catalog,
			 const TilesetCache& tilesets,
			 const MarkerSprites& markerSprites, QWidget* parent)
	: QWidget(parent),
	  catalog_(catalog),
	  tilesets_(tilesets),
	  markerSprites_(markerSprites),
	  markerEntries_{
		  {"estructuras_granja", "Cama (marcador)",
		   worldmodel::MarkerKind::Bed},
		  {"estructuras_granja", "Ventana general (marcador)",
		   worldmodel::MarkerKind::GeneralWindow},
		  {"granero", "Ventana granero (marcador)",
		   worldmodel::MarkerKind::CoopWindow},
	  } {
	categoryCombo_ = new QComboBox(this);
	categoryCombo_->setObjectName("editorCombo");
	for (const auto& [category, label] : catalog_.categories()) {
		categoryCombo_->addItem(QString::fromStdString(label),
					QString::fromStdString(category));
	}

	list_ = new PaletteListWidget("paletteList", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kPaletteMargins);
	layout->addWidget(categoryCombo_);
	layout->addWidget(list_, 1);

	connect(categoryCombo_, &QComboBox::currentIndexChanged, this,
		[this](int index) {
			if (index < 0) {
				list_->setCards({});
				return;
			}
			const std::string category =
				categoryCombo_->itemData(index)
					.toString()
					.toStdString();
			populateCategory(category);
		});

	connect(list_, &QListWidget::itemClicked, this,
		[this](QListWidgetItem* item) {
			if (item == nullptr) {
				return;
			}
			const QVariant data = item->data(Qt::UserRole);
			if (data.typeId() == QMetaType::Int) {
				emit markerSelected(
					static_cast<worldmodel::MarkerKind>(
						data.toInt()));
				return;
			}
			const std::string id = data.toString().toStdString();
			if (const auto portalKind =
				    portalPrefabs_.kindForPrefabId(id)) {
				const worldmodel::PrefabDef* prefab =
					catalog_.find(id);
				const int w = prefab != nullptr ? prefab->width : 1;
				const int h = prefab != nullptr ? prefab->height : 1;
				emit portalPicked(*portalKind, w, h);
				return;
			}
			if (const worldmodel::PrefabDef* prefab =
				    catalog_.find(id)) {
				emit prefabSelected(prefab);
			}
		});

	if (categoryCombo_->count() > 0) {
		categoryCombo_->setCurrentIndex(0);
		populateCategory(
			categoryCombo_->itemData(0).toString().toStdString());
	}
}

void PaletteView::clearSelection() {
	list_->clearSelection();
	list_->setCurrentItem(nullptr);
}

void PaletteView::populateCategory(const std::string& category) {
	std::vector<PaletteListWidget::Card> cards;

	for (const worldmodel::PrefabDef* prefab :
	     catalog_.byCategory(category)) {
		if (prefab == nullptr) {
			continue;
		}
		const QPixmap sprite = tilesets_.tilePixmap(
			prefab->tile, prefab->width, prefab->height);
		cards.push_back({
			tilesets_.centeredIcon(sprite, editor::kPaletteIconSize),
			QString::fromStdString(prefab->name),
			QString::fromStdString(prefab->id),
		});
	}
	for (const MarkerEntry& entry : markerEntries_) {
		if (entry.category != category) {
			continue;
		}
		cards.push_back({
			tilesets_.centeredIcon(
				markerSprites_.sprite(entry.kind),
				editor::kPaletteIconSize),
			entry.label,
			static_cast<int>(entry.kind),
		});
	}

	list_->setCards(cards);
}
