#pragma once

#include <string>

#include <QWidget>

#include "world_model/MapRules.h"
#include "world_model/World.h"

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;


class MapTreeWidget : public QWidget {
	Q_OBJECT
public:
	explicit MapTreeWidget(QWidget* parent = nullptr);


	void setExtraMaps(const worldmodel::World& world);

	void selectMap(const std::string& mapId);

signals:
	void mapSelected(const std::string& mapId);
	void addExtraMapRequested();
	void removeExtraMapRequested();
	void changeScenarioRequested();

private:
	void addLeaf(QTreeWidgetItem* group, const std::string& id);
	void populateInteriors(const worldmodel::World* world);

	worldmodel::MapRules rules_;
	QTreeWidget* tree_ = nullptr;
	QTreeWidgetItem* interiorsItem_ = nullptr;
	QTreeWidgetItem* extrasItem_ = nullptr;
	QPushButton* addButton_ = nullptr;
	QPushButton* removeButton_ = nullptr;
	QPushButton* changeScenarioButton_ = nullptr;
};
