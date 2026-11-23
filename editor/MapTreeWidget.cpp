#include "MapTreeWidget.h"

#include <vector>

#include <QIcon>
#include <QPushButton>
#include <QSize>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "Constants.h"
#include "world_model/Constants.h"

MapTreeWidget::MapTreeWidget(QWidget* parent) : QWidget(parent) {
	tree_ = new QTreeWidget(this);
	tree_->setObjectName("mapTree");
	tree_->setHeaderHidden(true);
	tree_->setIconSize(QSize(editor::kToolIconSize, editor::kToolIconSize));
	const auto addGroup = [this](const QString& title, const QString& icon,
				     const std::vector<std::string>& ids) {
		QTreeWidgetItem* group = new QTreeWidgetItem(tree_, {title});
		group->setIcon(0, QIcon(icon));
		for (const std::string& id : ids) {
			addLeaf(group, id);
		}
		return group;
	};
	addGroup("Exteriores", ":/editor/ui/tree_exteriors.png",
		 {"farm", "town"});
	interiorsItem_ =
		addGroup("Interiores", ":/editor/ui/tree_interiors.png", {});
	populateInteriors(nullptr);
	// Orden explicito 1..kDungeonLevels, no el alfanumerico de map
	std::vector<std::string> dungeons;
	for (int level = 1; level <= worldmodel::kDungeonLevels; ++level) {
		dungeons.push_back(rules_.dungeonId(level));
	}
	addGroup("Mazmorras", ":/editor/ui/tree_dungeons.png", dungeons);
	extrasItem_ = addGroup("Mapas extra", ":/editor/ui/tree_extras.png", {});
	tree_->expandAll();

	addButton_ = new QPushButton("+ Agregar mapa extra...", this);
	addButton_->setObjectName("editorButton");
	addButton_->setMinimumWidth(editor::kAddMapButtonMinWidth);
	removeButton_ = new QPushButton("Eliminar mapa extra", this);
	removeButton_->setObjectName("editorButton");
	removeButton_->setEnabled(false);
	changeScenarioButton_ =
		new QPushButton("Cambiar escenario base...", this);
	changeScenarioButton_->setObjectName("editorButton");

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

void MapTreeWidget::populateInteriors(const worldmodel::World* world) {
	// takeChildren no borra los items: hay que destruirlos a mano.
	qDeleteAll(interiorsItem_->takeChildren());
	if (world == nullptr) {
		addLeaf(interiorsItem_,
			rules_.cabinId(worldmodel::kMinPlayers));
	} else {
		for (int k = worldmodel::kMinPlayers;
		     k <= worldmodel::kMaxPlayers; ++k) {
			const std::string id = rules_.cabinId(k);
			if (world->maps.contains(id)) {
				addLeaf(interiorsItem_, id);
			}
		}
	}
	for (const std::string& id : rules_.fixedInteriorIds()) {
		addLeaf(interiorsItem_, id);
	}
}

void MapTreeWidget::setExtraMaps(const worldmodel::World& world) {
	populateInteriors(&world);
	qDeleteAll(extrasItem_->takeChildren());
	for (const auto& [id, map] : world.maps) {
		if (rules_.find(id) == nullptr) {
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
