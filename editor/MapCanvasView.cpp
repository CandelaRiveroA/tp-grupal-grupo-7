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
#include <map>

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

void MapCanvasView::setActivePortal(worldmodel::PortalKind kind, int width,
				    int height) {
	activePortalKind_ = kind;
	activePortalWidth_ = width;
	activePortalHeight_ = height;
	tool_ = EditorTool::PlacePortal;
	layerItem_->setPortalPreviewSize(width, height);
	layerItem_->setTool(EditorTool::PlacePortal);
	updateCursor();
}

void MapCanvasView::setPickTargetSize(int width, int height) {
	tool_ = EditorTool::PickTarget;
	layerItem_->setPortalPreviewSize(width, height);
	layerItem_->setTool(EditorTool::PickTarget);
	updateCursor();
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
		panMoved_ = false;
		panStartPos_ = event->pos();
		lastPanPos_ = event->pos();
		updateCursor();
		event->accept();
		return;
	}
	if (event->button() == Qt::LeftButton) {
		using ClickHandler = void (MapCanvasView::*)(worldmodel::TilePos);
		static const std::map<EditorTool, ClickHandler> kClickHandlers = {
			{EditorTool::Place, &MapCanvasView::placePrefabAt},
			{EditorTool::Erase, &MapCanvasView::eraseAt},
			{EditorTool::PlacePortal,
			 &MapCanvasView::placePortalAt},
			{EditorTool::PickTarget, &MapCanvasView::pickTargetAt},
		};
		const auto cell = cellAt(event->pos());
		const auto it = kClickHandlers.find(tool_);
		if (cell.has_value() && it != kClickHandlers.end()) {
			(this->*(it->second))(*cell);
		}
		event->accept();
		return;
	}
	QGraphicsView::mousePressEvent(event);
}

void MapCanvasView::mouseMoveEvent(QMouseEvent* event) {
	if (panning_) {
		if ((event->pos() - panStartPos_).manhattanLength() >=
		    editor::kPortalClickThresholdPx) {
			panMoved_ = true;
		}
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
		const bool clickInNavigate =
			event->button() == Qt::LeftButton && !panMoved_ &&
			tool_ == EditorTool::Navigate && !spacePressed_;
		panning_ = false;
		updateCursor();
		if (clickInNavigate) {
			openPortalAt(event->pos());
		}
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
	if (event->key() == Qt::Key_Escape) {
		if (tool_ == EditorTool::PickTarget) {
			emit pickTargetCancelled();
			event->accept();
			return;
		}
		if (tool_ == EditorTool::PlacePortal) {
			resetToNavigate();
			event->accept();
			return;
		}
	}
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
	const bool isCabinOutsideFarm =
		activePrefab_->role == worldmodel::ObjectRole::Cabin &&
		map_->id != editor::kHomeMapId;
	const worldmodel::ObjectRole role =
		(!activePrefab_->role.has_value() || isCabinOutsideFarm)
			? worldmodel::ObjectRole::Decoration
			: *activePrefab_->role;
	const std::optional<worldmodel::TilePos> door =
		role != worldmodel::ObjectRole::Decoration
			? prefabDoorAt(cell)
			: std::nullopt;
	map_->objects.push_back(worldmodel::MapObject{
		role,
		activePrefab_->tile,
		cell,
		activePrefab_->width,
		activePrefab_->height,
		anyBlocked,
		door,
		activePrefab_->collisionGrid,
	});
	notifyMapEdited();
}

void MapCanvasView::eraseAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	for (auto it = map_->portals.rbegin(); it != map_->portals.rend();
	     ++it) {
		if (it->kind != worldmodel::PortalKind::InteriorExit &&
		    containsCell(it->cell, it->width, it->height, cell)) {
			map_->portals.erase(std::next(it).base());
			notifyMapEdited();
			return;
		}
	}
	for (auto it = map_->objects.rbegin(); it != map_->objects.rend(); ++it) {
		const bool erasable =
			it->role == worldmodel::ObjectRole::Decoration ||
			(it->role == worldmodel::ObjectRole::Cabin &&
			 map_->id == editor::kHomeMapId);
		if (!erasable ||
		    !containsCell(it->cell, it->width, it->height, cell)) {
			continue;
		}
		const auto forwardIt = std::next(it).base();
		if (forwardIt->role == worldmodel::ObjectRole::Cabin) {
			int cabinIdx = 1;
			for (auto scan = map_->objects.begin();
			     scan != forwardIt; ++scan) {
				if (scan->role ==
				    worldmodel::ObjectRole::Cabin) {
					++cabinIdx;
				}
			}
			map_->objects.erase(forwardIt);
			emit cabinErased(cabinIdx);
			notifyMapEdited();
			return;
		}
		map_->objects.erase(forwardIt);
		notifyMapEdited();
		return;
	}
}

void MapCanvasView::placePortalAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	map_->portals.push_back(worldmodel::Portal{activePortalKind_, cell, "",
						   {}, activePortalWidth_,
						   activePortalHeight_});
	const std::size_t index = map_->portals.size() - 1;
	resetToNavigate();
	notifyMapEdited();
	emit portalTargetRequested(index);
}

void MapCanvasView::pickTargetAt(worldmodel::TilePos cell) {
	emit targetCellPicked(cell);
}

void MapCanvasView::openPortalAt(const QPoint& pos) {
	if (!map_) {
		return;
	}
	const auto cell = cellAt(pos);
	if (!cell.has_value()) {
		return;
	}
	for (std::size_t i = 0; i < map_->portals.size(); ++i) {
		const auto& portal = map_->portals[i];
		if (containsCell(portal.cell, portal.width, portal.height,
				 *cell)) {
			emit portalTargetRequested(i);
			return;
		}
	}
}

bool MapCanvasView::containsCell(worldmodel::TilePos origin, int width,
				 int height,
				 worldmodel::TilePos cell) const {
	return cell.column >= origin.column &&
	       cell.column < origin.column + width &&
	       cell.row >= origin.row && cell.row < origin.row + height;
}

std::optional<worldmodel::TilePos> MapCanvasView::prefabDoorAt(
	worldmodel::TilePos cell) const {
	const worldmodel::TilePos offset = activePrefab_->door.value_or(
		worldmodel::TilePos{activePrefab_->width / 2,
				    activePrefab_->height});
	return worldmodel::TilePos{cell.column + offset.column,
				   cell.row + offset.row};
}

void MapCanvasView::resetToNavigate() {
	tool_ = EditorTool::Navigate;
	layerItem_->setTool(EditorTool::Navigate);
	updateCursor();
	emit toolResetToNavigate();
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
