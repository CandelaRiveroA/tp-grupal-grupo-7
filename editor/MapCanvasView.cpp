#include "MapCanvasView.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <iterator>

#include "Constants.h"
#include "MapLayerItem.h"
#include "common/world_model/Map.h"
#include "common/world_model/MapObject.h"
#include "common/world_model/PrefabCatalog.h"

MapCanvasView::MapCanvasView(const TilesetCache& tilesets, QWidget* parent)
	: QGraphicsView(parent) {
	setRenderHint(QPainter::SmoothPixmapTransform, false);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	updateCursor();

	scene_ = new QGraphicsScene(this);
	setScene(scene_);

	layerItem_ = new MapLayerItem(tilesets);
	scene_->addItem(layerItem_);
}

void MapCanvasView::setMap(worldmodel::Map* map) {
	updateHoverCell(std::nullopt);
	map_ = map;
	layerItem_->setMap(map_);
	scene_->setSceneRect(layerItem_->boundingRect());
}

void MapCanvasView::setShowCollision(bool show) {
	layerItem_->setShowCollision(show);
}

void MapCanvasView::setTool(EditorTool tool) {
	tool_ = tool;
	layerItem_->setTool(tool);
	updateCursor();
}

void MapCanvasView::setActivePrefab(const worldmodel::PrefabDef* prefab) {
	activePrefab_ = prefab;
	layerItem_->setActivePrefab(prefab);
}

void MapCanvasView::zoomIn() {
	applyZoom(zoom_ * editor::kZoomStep);
}

void MapCanvasView::zoomOut() {
	applyZoom(zoom_ / editor::kZoomStep);
}

void MapCanvasView::resetZoom() {
	applyZoom(1.0);
}

void MapCanvasView::wheelEvent(QWheelEvent* event) {
	if (event->angleDelta().y() > 0) {
		zoomIn();
	} else if (event->angleDelta().y() < 0) {
		zoomOut();
	}
	event->accept();
}

void MapCanvasView::mousePressEvent(QMouseEvent* event) {
	const bool panTrigger = event->button() == Qt::MiddleButton ||
				(event->button() == Qt::LeftButton &&
				 (tool_ == EditorTool::Navigate || spacePressed_));
	if (panTrigger) {
		panning_ = true;
		lastPanPos_ = event->pos();
		updateCursor();
		event->accept();
		return;
	}
	if (event->button() == Qt::LeftButton) {
		const auto cell = cellAt(event->pos());
		if (cell.has_value()) {
			if (tool_ == EditorTool::Place) {
				placePrefabAt(*cell);
			} else if (tool_ == EditorTool::Erase) {
				eraseObjectAt(*cell);
			}
		}
		event->accept();
		return;
	}
	QGraphicsView::mousePressEvent(event);
}

void MapCanvasView::mouseMoveEvent(QMouseEvent* event) {
	if (panning_) {
		const QPoint delta = event->pos() - lastPanPos_;
		lastPanPos_ = event->pos();
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
						delta.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
		event->accept();
		return;
	}
	updateHoverCell(spacePressed_ ? std::nullopt : cellAt(event->pos()));
	QGraphicsView::mouseMoveEvent(event);
}

void MapCanvasView::mouseReleaseEvent(QMouseEvent* event) {
	if (panning_ && (event->button() == Qt::MiddleButton ||
			 event->button() == Qt::LeftButton)) {
		panning_ = false;
		updateCursor();
		event->accept();
		return;
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void MapCanvasView::leaveEvent(QEvent* event) {
	updateHoverCell(std::nullopt);
	QGraphicsView::leaveEvent(event);
}

void MapCanvasView::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
		spacePressed_ = true;
		updateHoverCell(std::nullopt);
		updateCursor();
		event->accept();
		return;
	}
	QGraphicsView::keyPressEvent(event);
}

void MapCanvasView::keyReleaseEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
		spacePressed_ = false;
		updateCursor();
		const QPoint cursorPos = viewport()->mapFromGlobal(QCursor::pos());
		if (!panning_ && viewport()->rect().contains(cursorPos)) {
			updateHoverCell(cellAt(cursorPos));
		}
		event->accept();
		return;
	}
	QGraphicsView::keyReleaseEvent(event);
}

void MapCanvasView::applyZoom(double targetZoom) {
	const double clamped =
		std::clamp(targetZoom, editor::kMinZoom, editor::kMaxZoom);
	const double factor = clamped / zoom_;
	if (factor != 1.0) {
		scale(factor, factor);
		zoom_ = clamped;
	}
}

std::optional<worldmodel::TilePos> MapCanvasView::cellAt(const QPoint& pos) const {
	if (!map_ || map_->tileSize <= 0) {
		return std::nullopt;
	}
	const QPointF scenePos = mapToScene(pos);
	const worldmodel::TilePos cell{
		static_cast<int>(std::floor(scenePos.x() / map_->tileSize)),
		static_cast<int>(std::floor(scenePos.y() / map_->tileSize)),
	};
	if (!map_->contains(cell)) {
		return std::nullopt;
	}
	return cell;
}

void MapCanvasView::updateHoverCell(std::optional<worldmodel::TilePos> cell) {
	if (hoverCell_ == cell) {
		return;
	}
	hoverCell_ = cell;
	layerItem_->setHoverCell(hoverCell_);
}

void MapCanvasView::placePrefabAt(worldmodel::TilePos cell) {
	if (!map_ || !activePrefab_ ||
	    !map_->fits(cell, activePrefab_->width, activePrefab_->height)) {
		return;
	}
	const bool anyBlocked = std::any_of(
		activePrefab_->collisionGrid.begin(),
		activePrefab_->collisionGrid.end(), [](bool b) { return b; });
	map_->objects.push_back(worldmodel::MapObject{
		worldmodel::ObjectRole::Decoration,
		activePrefab_->tile,
		cell,
		activePrefab_->width,
		activePrefab_->height,
		anyBlocked,
		std::nullopt,
		activePrefab_->collisionGrid,
	});
	notifyMapEdited();
}

void MapCanvasView::eraseObjectAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	for (auto it = map_->objects.rbegin(); it != map_->objects.rend(); ++it) {
		if (it->role != worldmodel::ObjectRole::Decoration) {
			continue;
		}
		if (cell.column >= it->cell.column &&
		    cell.column < it->cell.column + it->width &&
		    cell.row >= it->cell.row &&
		    cell.row < it->cell.row + it->height) {
			map_->objects.erase(std::next(it).base());
			notifyMapEdited();
			return;
		}
	}
}

void MapCanvasView::notifyMapEdited() {
	map_->rebuildCollision();
	layerItem_->update();
	emit mapEdited();
}

void MapCanvasView::updateCursor() {
	if (panning_) {
		viewport()->setCursor(Qt::ClosedHandCursor);
	} else if (tool_ == EditorTool::Navigate || spacePressed_) {
		viewport()->setCursor(Qt::OpenHandCursor);
	} else {
		viewport()->setCursor(Qt::CrossCursor);
	}
}
