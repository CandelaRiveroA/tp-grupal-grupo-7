#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <QWidget>

#include "TilesetCache.h"
#include "world_model/Geometry.h"
#include "world_model/MapRules.h"
#include "world_model/Portal.h"
#include "world_model/PrefabCatalog.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"
#include "world_model/WorldRepository.h"

class MapCanvasView;
class MapTreeWidget;
class PaletteView;
class PortalsView;
class ValidationPanel;
class QButtonGroup;
class QDialog;
class QHBoxLayout;
class QLabel;
class QTabWidget;
class QToolButton;

// Mundo en memoria, mapa activo, mapas extra por demanda.
class WorkspaceView : public QWidget {
	Q_OBJECT
public:
	WorkspaceView(const worldmodel::ScenarioLibrary& scenarios,
		      const worldmodel::WorldRepository& worlds,
		      QWidget* parent = nullptr);

	void setWorld(const worldmodel::World& world);
	const worldmodel::PrefabCatalog& catalog() const;

signals:
	void backRequested();

private:
	struct PendingPortalTarget {
		std::string originMapId;
		std::size_t portalIndex = 0;
	};

	const worldmodel::ScenarioLibrary& scenarios_;
	const worldmodel::WorldRepository& worlds_;
	worldmodel::PrefabCatalog catalog_;
	TilesetCache tilesets_;
	worldmodel::World world_;
	worldmodel::MapRules rules_;
	std::string activeMapId_;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	std::optional<int> pendingErasedCabin_;
	std::optional<PendingPortalTarget> pendingPortal_;
	bool dirty_ = false;

	QLabel* heading_ = nullptr;
	QButtonGroup* toolGroup_ = nullptr;
	QToolButton* navigateButton_ = nullptr;
	QToolButton* placeButton_ = nullptr;
	QToolButton* eraseButton_ = nullptr;
	QToolButton* moveButton_ = nullptr;
	MapTreeWidget* tree_ = nullptr;
	MapCanvasView* canvas_ = nullptr;
	ValidationPanel* validationPanel_ = nullptr;
	QTabWidget* rightTabs_ = nullptr;
	PaletteView* palette_ = nullptr;
	PortalsView* portals_ = nullptr;
	QDialog* validationDialog_ = nullptr;

	QHBoxLayout* buildTopBar();
	QToolButton* makeToolButton(const QString& text, const QString& icon,
				    bool checkable);
	std::vector<const worldmodel::Scenario*> scenarioOptions() const;
	void selectMap(const std::string& mapId);
	void validateNow();
	void saveNow();
	void addExtraMap();
	void removeExtraMap();
	void replaceScenario();
	void onPrefabSelected(const worldmodel::PrefabDef* prefab);
	void setActivePrefab(const worldmodel::PrefabDef* prefab);
	void onPortalPicked(worldmodel::PortalKind kind, int width, int height);
	void onPortalTargetRequested(std::size_t portalIndex);
	void onTargetCellPicked(worldmodel::TilePos cell);
	void onPickTargetCancelled();
	void syncFarmCabins(std::optional<int> erasedCabinIndex);
	void uncheckToolButtons();
	void setPickTargetUiLocked(bool locked);
	void setDirty();
	void updateHeading();
	bool hasTilesetConflict(const std::string& tilesetId) const;
};
