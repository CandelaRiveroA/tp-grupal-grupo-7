#pragma once

#include <QWidget>
#include <string>

#include "PortalPrefabs.h"
#include "world_model/Portal.h"
#include "world_model/PrefabCatalog.h"

class QComboBox;
class QListWidget;
class TilesetCache;

// Panel de prefabs con selector de categoria y previsualizacion completa.
class PaletteView : public QWidget {
	Q_OBJECT
public:
	PaletteView(const worldmodel::PrefabCatalog& catalog,
		    const TilesetCache& tilesets, QWidget* parent = nullptr);

	void clearSelection();

signals:
	void prefabSelected(const worldmodel::PrefabDef* prefab);
	void bedSelected();
	void portalPicked(worldmodel::PortalKind kind, int width, int height);

private:
	void populateCategory(const std::string& category);

	const worldmodel::PrefabCatalog& catalog_;
	const TilesetCache& tilesets_;
	PortalPrefabs portalPrefabs_;
	QComboBox* categoryCombo_ = nullptr;
	QListWidget* list_ = nullptr;
};
