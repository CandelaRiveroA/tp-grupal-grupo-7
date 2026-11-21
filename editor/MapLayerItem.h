#pragma once

#include <QGraphicsItem>
#include <map>
#include <optional>

#include "common/world_model/Geometry.h"
#include "common/world_model/Tile.h"

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class TilesetCache;

namespace worldmodel {
struct Map;
struct PrefabDef;
}

enum class EditorTool { Navigate, Place, Erase, PlacePortal, PickTarget };

// Item unico que dibuja todas las capas visibles del mapa en orden Z.
class MapLayerItem : public QGraphicsItem {
public:
	explicit MapLayerItem(const TilesetCache& tilesets);

	void setMap(const worldmodel::Map* map);
	void setShowCollision(bool show);
	void setTool(EditorTool tool);
	void setActivePrefab(const worldmodel::PrefabDef* prefab);
	void setPortalPreviewSize(int width, int height);
	void setHoverCell(std::optional<worldmodel::TilePos> cell);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
		   QWidget* widget) override;

private:
	bool hasValidMap() const;
	bool hasPreview() const;
	bool hasPrefabPreview() const;
	bool alwaysPreview() const;
	void drawPlacementPreview(QPainter* painter, int ts) const;
	void drawGridLayer(QPainter* painter, const std::vector<std::optional<worldmodel::TileRef>>& grid,
			   int ts, int minCol, int maxCol, int minRow,
			   int maxRow) const;
	void drawObjects(QPainter* painter, int ts, int minCol, int maxCol,
			 int minRow, int maxRow) const;
	void drawMarkersAndPortals(QPainter* painter, int ts, int minCol,
				   int maxCol, int minRow, int maxRow) const;
	void drawCellGrid(QPainter* painter, int ts, int minCol, int maxCol,
			  int minRow, int maxRow) const;
	void drawCollisionOverlay(QPainter* painter, int ts, int minCol,
				  int maxCol, int minRow, int maxRow) const;

	const TilesetCache& tilesets_;
	std::map<EditorTool, bool (MapLayerItem::*)() const> previewChecks_;
	const worldmodel::Map* map_ = nullptr;
	bool showCollision_ = false;
	EditorTool tool_ = EditorTool::Navigate;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	int portalPreviewW_ = 1;
	int portalPreviewH_ = 1;
	std::optional<worldmodel::TilePos> hoverCell_;
};
