#include "PaletteView.h"

#include <QComboBox>
#include <QIcon>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QSignalBlocker>
#include <QSize>
#include <QString>
#include <QVBoxLayout>

#include "Constants.h"
#include "TilesetCache.h"
#include "world_model/PrefabCatalog.h"

PaletteView::PaletteView(const worldmodel::PrefabCatalog& catalog,
			 const TilesetCache& tilesets, QWidget* parent)
	: QWidget(parent), catalog_(catalog), tilesets_(tilesets) {
	categoryCombo_ = new QComboBox(this);
	categoryCombo_->setObjectName("editorCombo");
	for (const auto& [category, label] : catalog_.categories()) {
		categoryCombo_->addItem(QString::fromStdString(label),
					QString::fromStdString(category));
	}

	list_ = new QListWidget(this);
	list_->setObjectName("paletteList");
	list_->setViewMode(QListView::IconMode);
	list_->setMovement(QListView::Static);
	list_->setResizeMode(QListView::Adjust);
	list_->setWrapping(true);
	list_->setWordWrap(true);
	list_->setUniformItemSizes(true);
	list_->setIconSize(
		QSize(editor::kPaletteIconSize, editor::kPaletteIconSize));
	list_->setGridSize(editor::kPaletteGridSize);
	list_->setSpacing(editor::kPaletteItemSpacing);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kPaletteMargins);
	layout->addWidget(categoryCombo_);
	layout->addWidget(list_, 1);

	connect(categoryCombo_, &QComboBox::currentIndexChanged, this,
		[this](int index) {
			if (index < 0) {
				const QSignalBlocker blocker(list_);
				list_->clear();
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
			const std::string id =
				item->data(Qt::UserRole).toString().toStdString();
			if (id == "__bed__") {
				emit bedSelected();
				return;
			}
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
	const QSignalBlocker blocker(list_);
	list_->clearSelection();
	list_->setCurrentItem(nullptr);
}

void PaletteView::populateCategory(const std::string& category) {
	const QSignalBlocker blocker(list_);
	list_->clear();

	for (const worldmodel::PrefabDef* prefab :
	     catalog_.byCategory(category)) {
		if (prefab == nullptr) {
			continue;
		}
		const QPixmap sprite = tilesets_.tilePixmap(
			prefab->tile, prefab->width, prefab->height);
		const QString label = QString("%1 (%2x%3)")
					      .arg(QString::fromStdString(
						      prefab->name))
					      .arg(prefab->width)
					      .arg(prefab->height);
		auto* item = new QListWidgetItem(
			tilesets_.centeredIcon(sprite, editor::kPaletteIconSize),
			label, list_);
		item->setToolTip(label);
		item->setData(Qt::UserRole, QString::fromStdString(prefab->id));
	}
	if (category == "estructuras_granja") {
		auto* bedItem = new QListWidgetItem(
			tilesets_.centeredIcon(
				QPixmap(":/editor/ui/bed_marker.png"),
				editor::kPaletteIconSize),
			"Cama (marcador)", list_);
		bedItem->setToolTip("Cama (marcador)");
		bedItem->setData(Qt::UserRole, QString("__bed__"));
	}
}
