#pragma once

#include <string>
#include <vector>

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include "world_model/MapRules.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"
#include "world_model/WorldRepository.h"

class MapPlaceholderView;
class MapTreeWidget;
class PaletteView;
class ValidationPanel;
class QDialog;

// Mundo en memoria, mapa activo, mapas extra por demanda.
class WorkspaceView : public QWidget {
	Q_OBJECT
public:
	WorkspaceView(const worldmodel::ScenarioLibrary& scenarios,
		      const worldmodel::WorldRepository& worlds,
		      QWidget* parent = nullptr);

	void setWorld(const worldmodel::World& world);

signals:
	void backRequested();

private:
	QHBoxLayout* buildTopBar();
	std::vector<const worldmodel::Scenario*> scenarioOptions() const;
	void selectMap(const std::string& mapId);
	void validateNow();
	void saveNow();
	void addExtraMap();
	void removeExtraMap();
	void replaceScenario();
	void setDirty();
	void updateHeading();

	const worldmodel::ScenarioLibrary& scenarios_;
	const worldmodel::WorldRepository& worlds_;
	worldmodel::World world_;
	worldmodel::MapRules rules_;
	std::string activeMapId_;
	bool dirty_ = false;

	QLabel* heading_ = nullptr;
	MapTreeWidget* tree_ = nullptr;
	MapPlaceholderView* placeholder_ = nullptr;
	ValidationPanel* validationPanel_ = nullptr;
	PaletteView* palette_ = nullptr;
	QDialog* validationDialog_ = nullptr;
};
