#pragma once

#include <QGraphicsView>
#include <QPoint>
#include <cstddef>
#include <map>
#include <optional>

#include "MapLayerItem.h"
#include "common/world_model/Geometry.h"
#include "common/world_model/MapObject.h"
#include "common/world_model/MapRules.h"
#include "common/world_model/Portal.h"

class QEvent;
class QKeyEvent;
class TilesetCache;

namespace worldmodel {
struct Map;
struct PrefabDef;
}

// Vista 2D del mapa activo con zoom, paneo, grilla, colision y edicion de prefabs.
class MapCanvasView : public QGraphicsView {
	Q_OBJECT

public:
	explicit MapCanvasView(const TilesetCache& tilesets, QWidget* parent = nullptr);

	void setMap(worldmodel::Map* map);
	void setShowCollision(bool show);
	void setTool(EditorTool tool);
	void setActivePrefab(const worldmodel::PrefabDef* prefab);
	void setActivePortal(worldmodel::PortalKind kind, int width, int height);
	void setPickTargetSize(int width, int height);
	void zoomIn();
	void zoomOut();
	void resetZoom();

signals:
	void mapEdited();
	void cabinErased(int cabinIndex);
	void portalTargetRequested(std::size_t portalIndex);
	void targetCellPicked(worldmodel::TilePos cell);
	void pickTargetCancelled();
	void toolResetToNavigate();

protected:
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	void applyZoom(double targetZoom);
	std::optional<worldmodel::TilePos> cellAt(const QPoint& pos) const;
	void updateHoverCell(std::optional<worldmodel::TilePos> cell);
	void placePrefabAt(worldmodel::TilePos cell);
	void eraseAt(worldmodel::TilePos cell);
	void moveAt(worldmodel::TilePos cell);
	std::optional<MoveTarget> pickMoveTarget(worldmodel::TilePos cell) const;
	void moveObject(std::size_t index, worldmodel::TilePos cell);
	void moveMarker(std::size_t index, worldmodel::TilePos cell);
	void movePortal(std::size_t index, worldmodel::TilePos cell);
	void clearMoveTarget();
	void placePortalAt(worldmodel::TilePos cell);
	void pickTargetAt(worldmodel::TilePos cell);
	void openPortalAt(const QPoint& pos);
	bool containsCell(worldmodel::TilePos origin, int width, int height,
			  worldmodel::TilePos cell) const;
	std::optional<worldmodel::TilePos> prefabDoorAt(
		worldmodel::TilePos cell) const;
	void resetToNavigate();
	void notifyMapEdited();
	void updateCursor();

	QGraphicsScene* scene_ = nullptr;
	MapLayerItem* layerItem_ = nullptr;
	worldmodel::Map* map_ = nullptr;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	std::optional<MoveTarget> moveTarget_;
	std::map<EditorTool, void (MapCanvasView::*)(worldmodel::TilePos)>
		clickHandlers_;
	std::map<MoveKind,
		 void (MapCanvasView::*)(std::size_t, worldmodel::TilePos)>
		moveAppliers_;
	worldmodel::MapRules rules_;
	worldmodel::PortalKind activePortalKind_ = worldmodel::PortalKind::Link;
	int activePortalWidth_ = 1;
	int activePortalHeight_ = 1;
	EditorTool tool_ = EditorTool::Navigate;
	std::optional<worldmodel::TilePos> hoverCell_;
	double zoom_ = 1.0;
	bool panning_ = false;
	bool panMoved_ = false;
	bool spacePressed_ = false;
	QPoint panStartPos_;
	QPoint lastPanPos_;
};
