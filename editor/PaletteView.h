#pragma once

#include <QString>
#include <QWidget>
#include <string>
#include <vector>

#include "PortalPrefabs.h"
#include "world_model/Marker.h"
#include "world_model/Portal.h"
#include "world_model/PrefabCatalog.h"

class MarkerSprites;
class PaletteListWidget;
class QComboBox;
class TilesetCache;

// Panel de prefabs con selector de categoria y previsualizacion completa.
class PaletteView : public QWidget {
	Q_OBJECT
public:
	PaletteView(const worldmodel::PrefabCatalog& catalog,
		    const TilesetCache& tilesets,
		    const MarkerSprites& markerSprites,
		    QWidget* parent = nullptr);

	void clearSelection();

signals:
	void prefabSelected(const worldmodel::PrefabDef* prefab);
	void markerSelected(worldmodel::MarkerKind kind);
	void portalPicked(worldmodel::PortalKind kind, int width, int height);

private:
	struct MarkerEntry {
		std::string category;
		QString label;
		worldmodel::MarkerKind kind;
	};

	void populateCategory(const std::string& category);

	const worldmodel::PrefabCatalog& catalog_;
	const TilesetCache& tilesets_;
	const MarkerSprites& markerSprites_;
	PortalPrefabs portalPrefabs_;
	std::vector<MarkerEntry> markerEntries_;
	QComboBox* categoryCombo_ = nullptr;
	PaletteListWidget* list_ = nullptr;
};
