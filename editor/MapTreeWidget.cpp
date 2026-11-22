#include "MapTreeWidget.h"

#include <vector>

#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "Constants.h"
#include "world_model/MapRules.h"

MapTreeWidget::MapTreeWidget(QWidget* parent) : QWidget(parent) {
	tree_ = new QTreeWidget(this);
	tree_->setHeaderHidden(true);
	const auto addGroup = [this](const QString& title,
				     const std::vector<std::string>& ids) {
		QTreeWidgetItem* group = new QTreeWidgetItem(tree_, {title});
		for (const std::string& id : ids) {
			addLeaf(group, id);
		}
		return group;
	};
	addGroup("Exteriores", {"farm", "town"});
	interiorsItem_ = addGroup("Interiores",
				  {"cabin_1", "carpentry", "blacksmith",
				   "general_store", "ranch"});
	// Orden explicito 1..10, no el alfanumerico de map
	const worldmodel::MapRules rules;
	std::vector<std::string> dungeons;
	for (int level = 1; level <= 10; ++level) {
		dungeons.push_back(rules.dungeonId(level));
	}
	addGroup("Mazmorras", dungeons);
	extrasItem_ = addGroup("Mapas extra", {});
	tree_->expandAll();

	addButton_ = new QPushButton("+ Agregar mapa extra...", this);
	addButton_->setMinimumWidth(editor::kAddMapButtonMinWidth);
	removeButton_ = new QPushButton("Eliminar mapa extra", this);
	removeButton_->setEnabled(false);
	changeScenarioButton_ =
		new QPushButton("Cambiar escenario base...", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(tree_);
	layout->addWidget(addButton_);
	layout->addWidget(removeButton_);
	layout->addWidget(changeScenarioButton_);

	connect(addButton_, &QPushButton::clicked, this,
		&MapTreeWidget::addExtraMapRequested);
	connect(removeButton_, &QPushButton::clicked, this,
		&MapTreeWidget::removeExtraMapRequested);
	connect(changeScenarioButton_, &QPushButton::clicked, this,
		&MapTreeWidget::changeScenarioRequested);
	connect(tree_, &QTreeWidget::itemClicked, this,
		[this](QTreeWidgetItem* item) {
			// Los grupos no tienen padre: solo las hojas son mapas.
			if (item->parent() != nullptr) {
				emit mapSelected(item->text(0).toStdString());
			}
		});
}

void MapTreeWidget::addLeaf(QTreeWidgetItem* group, const std::string& id) {
	new QTreeWidgetItem(group, {QString::fromStdString(id)});
}

void MapTreeWidget::setExtraMaps(const worldmodel::World& world) {
	// takeChildren no borra los items: hay que destruirlos a mano.
	qDeleteAll(interiorsItem_->takeChildren());
	qDeleteAll(extrasItem_->takeChildren());
	const worldmodel::MapRules rules;
	for (int k = worldmodel::kMinPlayers; k <= worldmodel::kMaxPlayers;
	     ++k) {
		const std::string id = rules.cabinId(k);
		if (world.maps.contains(id)) {
			addLeaf(interiorsItem_, id);
		}
	}
	for (const char* id :
	     {"carpentry", "blacksmith", "general_store", "ranch"}) {
		addLeaf(interiorsItem_, id);
	}
	for (const auto& [id, map] : world.maps) {
		if (rules.find(id) == nullptr) {
			addLeaf(extrasItem_, id);
		}
	}
}

void MapTreeWidget::selectMap(const std::string& mapId) {
	const QList<QTreeWidgetItem*> matches =
		tree_->findItems(QString::fromStdString(mapId),
				 Qt::MatchExactly | Qt::MatchRecursive, 0);
	QTreeWidgetItem* item = matches.isEmpty() ? nullptr : matches.first();
	if (item != nullptr) {
		tree_->setCurrentItem(item, 0);
	}
	removeButton_->setEnabled(item != nullptr &&
				  item->parent() == extrasItem_);
}
