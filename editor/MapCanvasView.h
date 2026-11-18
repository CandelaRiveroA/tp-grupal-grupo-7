#pragma once

#include <QGraphicsView>
#include <QPoint>
#include <optional>

#include "MapLayerItem.h"
#include "common/world_model/Geometry.h"

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
	void zoomIn();
	void zoomOut();
	void resetZoom();

signals:
	void mapEdited();

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
	void eraseObjectAt(worldmodel::TilePos cell);
	void notifyMapEdited();
	void updateCursor();

	QGraphicsScene* scene_ = nullptr;
	MapLayerItem* layerItem_ = nullptr;
	worldmodel::Map* map_ = nullptr;
	const worldmodel::PrefabDef* activePrefab_ = nullptr;
	EditorTool tool_ = EditorTool::Navigate;
	std::optional<worldmodel::TilePos> hoverCell_;
	double zoom_ = 1.0;
	bool panning_ = false;
	bool spacePressed_ = false;
	QPoint lastPanPos_;
};
