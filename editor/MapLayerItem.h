#pragma once

#include <QGraphicsItem>
#include <QPixmap>
#include <cstddef>
#include <map>
#include <optional>

#include "PortalPrefabs.h"
#include "common/world_model/Geometry.h"
#include "common/world_model/Marker.h"
#include "common/world_model/Portal.h"
#include "common/world_model/Tile.h"

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class TilesetCache;

namespace worldmodel {
struct Map;
struct MapObject;
class PrefabCatalog;
struct PrefabDef;
}

enum class EditorTool { Navigate, Place, Erase, Move, PlacePortal, PickTarget };

enum class MoveKind { Object, Marker, Portal };

struct MoveTarget {
	MoveKind kind = MoveKind::Object;
	std::size_t index = 0;
};

// Item unico que dibuja todas las capas visibles del mapa en orden Z.
class MapLayerItem : public QGraphicsItem {
public:
	MapLayerItem(const worldmodel::PrefabCatalog& catalog,
		     const TilesetCache& tilesets);

	void setMap(const worldmodel::Map* map);
	void setShowCollision(bool show);
	void setTool(EditorTool tool);
	void setActivePrefab(const worldmodel::PrefabDef* prefab);
	void setPlacingBed(bool placing);
	void setMoveTarget(std::optional<MoveTarget> target);
	void setPortalPreview(worldmodel::PortalKind kind, int width,
			      int height);
	void setHoverCell(std::optional<worldmodel::TilePos> cell);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
		   QWidget* widget) override;

private:
	const worldmodel::PrefabCatalog& catalog_;
	const TilesetCache& tilesets_;
	PortalPrefabs portalPrefabs_;
	QPixmap defaultMarkerSprite_;
	std::map<worldmodel::MarkerKind, QPixmap> markerSprites_;
	QPixmap portalSprite_;
	std::map<EditorTool, bool (MapLayerItem::*)() const> previewChecks_;
	std::map<EditorTool, void (MapLayerItem::*)(QPainter*, int) const>
		previewDrawers_;
	std::map<MoveKind, void (MapLayerItem::*)(QPainter*, int) const>
		movePreviewDrawers_;
	const worldmodel::Map* map_ = nullptr;
	bool showCollision_ = false;
	EditorTool tool_ = EditorTool::Navigate;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	bool placingBed_ = false;
	std::optional<MoveTarget> moveTarget_;
	worldmodel::PortalKind portalPreviewKind_ = worldmodel::PortalKind::Link;
	int portalPreviewW_ = 1;
	int portalPreviewH_ = 1;
	std::optional<worldmodel::TilePos> hoverCell_;

	bool hasValidMap() const;
	bool hasPreview() const;
	bool hasPrefabPreview() const;
	bool hasMovePreview() const;
	bool alwaysPreview() const;
	void drawPlacementPreview(QPainter* painter, int ts) const;
	void drawPrefabPreview(QPainter* painter, int ts) const;
	void drawMovePreview(QPainter* painter, int ts) const;
	void drawMoveObjectPreview(QPainter* painter, int ts) const;
	void drawMoveMarkerPreview(QPainter* painter, int ts) const;
	void drawMovePortalPreview(QPainter* painter, int ts) const;
	void drawPlacePortalPreview(QPainter* painter, int ts) const;
	void drawPortalPreviewBox(QPainter* painter, int ts,
				  worldmodel::PortalKind kind, int width,
				  int height) const;
	void drawPickTargetPreview(QPainter* painter, int ts) const;
	void drawMarkersAndPortals(QPainter* painter, int ts, int minCol,
				   int maxCol, int minRow, int maxRow) const;
	void drawCellGrid(QPainter* painter, int ts, int minCol, int maxCol,
			  int minRow, int maxRow) const;
	void drawCollisionOverlay(QPainter* painter, int ts, int minCol,
				  int maxCol, int minRow, int maxRow) const;
	int markerHeight(worldmodel::MarkerKind kind) const;
};
