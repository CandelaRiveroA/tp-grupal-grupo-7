#pragma once

#include <string>
#include <vector>

#include <QWidget>

#include "TilesetCache.h"
#include "world_model/MapRules.h"
#include "world_model/PrefabCatalog.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"
#include "world_model/WorldRepository.h"

class MapCanvasView;
class MapTreeWidget;
class PaletteView;
class ValidationPanel;
class QDialog;
class QHBoxLayout;
class QLabel;
class QToolButton;

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
	void onPrefabSelected(const worldmodel::PrefabDef* prefab);
	void setActivePrefab(const worldmodel::PrefabDef* prefab);
	void setDirty();
	void updateHeading();
	bool hasTilesetConflict(const std::string& tilesetId) const;

	const worldmodel::ScenarioLibrary& scenarios_;
	const worldmodel::WorldRepository& worlds_;
	worldmodel::PrefabCatalog catalog_;
	TilesetCache tilesets_;
	worldmodel::World world_;
	worldmodel::MapRules rules_;
	std::string activeMapId_;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	bool dirty_ = false;

	QLabel* heading_ = nullptr;
	QToolButton* placeButton_ = nullptr;
	MapTreeWidget* tree_ = nullptr;
	MapCanvasView* canvas_ = nullptr;
	ValidationPanel* validationPanel_ = nullptr;
	PaletteView* palette_ = nullptr;
	QDialog* validationDialog_ = nullptr;
};
