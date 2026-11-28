#include "MapCanvasView.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QScrollBar>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>

#include "Constants.h"
#include "MapLayerItem.h"
#include "common/world_model/Constants.h"
#include "common/world_model/Map.h"
#include "common/world_model/MapObject.h"
#include "common/world_model/PrefabCatalog.h"

MapCanvasView::MapCanvasView(const worldmodel::PrefabCatalog& catalog,
			     const TilesetCache& tilesets,
			     const MarkerSprites& markerSprites,
			     QWidget* parent)
	: QGraphicsView(parent),
	  catalog_(catalog),
	  disallowedMarkerMessages_{
		  {worldmodel::MarkerKind::Bed,
		   "La cama solo puede colocarse en el interior de una "
		   "cabana."},
	  },
	  clickHandlers_{
		  {EditorTool::Place, &MapCanvasView::placePrefabAt},
		  {EditorTool::Erase, &MapCanvasView::eraseAt},
		  {EditorTool::Move, &MapCanvasView::moveAt},
		  {EditorTool::PlacePortal, &MapCanvasView::placePortalAt},
		  {EditorTool::PickTarget, &MapCanvasView::pickTargetAt},
	  },
	  moveAppliers_{
		  {MoveKind::Object, &MapCanvasView::moveObject},
		  {MoveKind::Marker, &MapCanvasView::moveMarker},
		  {MoveKind::Portal, &MapCanvasView::movePortal},
	  } {
	setRenderHint(QPainter::SmoothPixmapTransform, false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	updateCursor();

	scene_ = new QGraphicsScene(this);
	setScene(scene_);
	setBackgroundBrush(QPixmap(":/editor/ui/editor_canvas_bg.png"));

	layerItem_ = new MapLayerItem(catalog_, tilesets, markerSprites);
	scene_->addItem(layerItem_);
}

void MapCanvasView::setMap(worldmodel::Map* map) {
	updateHoverCell(std::nullopt);
	clearMoveTarget();
	map_ = map;
	layerItem_->setMap(map_);
	scene_->setSceneRect(layerItem_->boundingRect());
}

void MapCanvasView::setShowCollision(bool show) {
	layerItem_->setShowCollision(show);
}

void MapCanvasView::setTool(EditorTool tool) {
	if (tool != EditorTool::Move) {
		clearMoveTarget();
	}
	tool_ = tool;
	layerItem_->setTool(tool);
	updateCursor();
}

void MapCanvasView::setActivePrefab(const worldmodel::PrefabDef* prefab) {
	if (prefab != nullptr) {
		placingMarker_.reset();
	}
	activePrefab_ = prefab;
	layerItem_->setActivePrefab(prefab);
}

void MapCanvasView::setPlacingMarker(std::optional<worldmodel::MarkerKind> kind) {
	if (kind.has_value()) {
		activePrefab_ = nullptr;
	}
	placingMarker_ = kind;
	layerItem_->setPlacingMarker(kind);
}

void MapCanvasView::clearPlacement() {
	activePrefab_ = nullptr;
	placingMarker_.reset();
	layerItem_->clearPlacement();
}

void MapCanvasView::setActivePortal(worldmodel::PortalKind kind, int width,
				    int height) {
	activePortalKind_ = kind;
	activePortalWidth_ = width;
	activePortalHeight_ = height;
	layerItem_->setPortalPreview(kind, width, height);
	setTool(EditorTool::PlacePortal);
}

void MapCanvasView::setPickTargetSize(int width, int height) {
	layerItem_->setPortalPreview(worldmodel::PortalKind::Link, width,
				     height);
	setTool(EditorTool::PickTarget);
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
		const auto cell = cellAt(event->pos());
		const auto it = clickHandlers_.find(tool_);
		if (cell.has_value() && it != clickHandlers_.end()) {
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
		if (tool_ == EditorTool::Move && moveTarget_.has_value()) {
			clearMoveTarget();
			event->accept();
			return;
		}
		if (tool_ == EditorTool::PlacePortal ||
		    tool_ == EditorTool::Move) {
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

bool MapCanvasView::validateMarkerPlacement(worldmodel::MarkerKind kind,
					    worldmodel::TilePos cell) {
	if (!map_->contains(cell)) {
		return false;
	}
	if (!rules_.markerAllowedIn(map_->id, kind)) {
		const auto it = disallowedMarkerMessages_.find(kind);
		const QString msg =
			it != disallowedMarkerMessages_.end()
				? it->second
				: "Este marcador solo puede colocarse en "
				  "mapas interiores.";
		emit placementRejected("Ubicacion no permitida", msg);
		return false;
	}
	const worldmodel::MarkerSpec spec = rules_.markerSpec(kind);
	if (!map_->fits(cell, 1, spec.height) ||
	    (spec.requiresWalkable &&
	     !map_->isAreaWalkable(cell, 1, spec.height))) {
		emit placementRejected(
			"Ubicacion no valida",
			spec.requiresWalkable
				? "El marcador no entra o requiere celdas "
				  "transitables."
				: "El marcador no entra dentro del mapa.");
		return false;
	}
	return true;
}

void MapCanvasView::placeMarkerAt(worldmodel::MarkerKind kind,
				  worldmodel::TilePos cell) {
	if (!validateMarkerPlacement(kind, cell)) {
		return;
	}
	if (rules_.isMarkerUnique(map_->id, kind)) {
		for (auto& marker : map_->markers) {
			if (marker.kind == kind) {
				marker.cell = cell;
				notifyMapEdited();
				return;
			}
		}
	}
	map_->markers.push_back({kind, cell});
	notifyMapEdited();
}

bool MapCanvasView::canPlacePrefab(const worldmodel::PrefabDef& prefab) {
	if (!prefab.role.has_value()) {
		return true;
	}
	const worldmodel::ObjectRole role = *prefab.role;
	if (role == worldmodel::ObjectRole::Cabin) {
		if (map_->id != editor::kHomeMapId) {
			emit placementRejected(
				"Ubicacion no permitida",
				"Las cabanas solo pueden colocarse en la "
				"granja (farm).");
			return false;
		}
		const int cabins = static_cast<int>(std::count_if(
			map_->objects.begin(), map_->objects.end(),
			[](const worldmodel::MapObject& obj) {
				return obj.role ==
				       worldmodel::ObjectRole::Cabin;
			}));
		if (cabins >= worldmodel::kMaxPlayers) {
			emit placementRejected(
				"Limite de cabanas",
				"No se pueden colocar mas de 4 cabanas en la "
				"granja.");
			return false;
		}
	}
	const bool isShop = role == worldmodel::ObjectRole::Carpentry ||
			    role == worldmodel::ObjectRole::Blacksmith ||
			    role == worldmodel::ObjectRole::GeneralStore ||
			    role == worldmodel::ObjectRole::Ranch;
	if (isShop && map_->id != "town") {
		emit placementRejected(
			"Ubicacion no permitida",
			"Las casas de los comerciantes solo pueden colocarse "
			"en el pueblo (town).");
		return false;
	}
	const std::optional<std::string> requiredMap =
		rules_.requiredHostMap(role);
	if (requiredMap.has_value() && map_->id != *requiredMap) {
		emit placementRejected(
			"Ubicacion no permitida",
			QString::fromStdString(
				"Este edificio solo puede colocarse en " +
				*requiredMap + "."));
		return false;
	}
	return true;
}

void MapCanvasView::placePrefabAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	if (placingMarker_.has_value()) {
		placeMarkerAt(*placingMarker_, cell);
		return;
	}
	if (!activePrefab_ ||
	    !map_->fits(cell, activePrefab_->width, activePrefab_->height) ||
	    !canPlacePrefab(*activePrefab_)) {
		return;
	}
	const worldmodel::ObjectRole role =
		activePrefab_->role.value_or(worldmodel::ObjectRole::Decoration);
	const std::optional<worldmodel::TilePos> door =
		role != worldmodel::ObjectRole::Decoration &&
				role != worldmodel::ObjectRole::Building
			? prefabDoorAt(cell)
			: std::nullopt;
	const int doorWidth =
		door.has_value() ? activePrefab_->doorWidth : 1;
	map_->objects.push_back(
		makeObjectFromPrefab(*activePrefab_, cell, door, doorWidth));
	notifyMapEdited();
}

void MapCanvasView::eraseAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	for (auto it = map_->markers.rbegin(); it != map_->markers.rend();
	     ++it) {
		const worldmodel::MarkerSpec spec = rules_.markerSpec(it->kind);
		if (spec.erasable &&
		    containsCell(it->cell, 1, spec.height, cell)) {
			map_->markers.erase(std::next(it).base());
			notifyMapEdited();
			return;
		}
	}
	for (auto it = map_->portals.rbegin(); it != map_->portals.rend();
	     ++it) {
		if (it->kind != worldmodel::PortalKind::InteriorExit &&
		    containsCell(it->cell, it->width, it->height, cell)) {
			const auto linkedIdx = linkedPortalObjectIndex(*it);
			map_->portals.erase(std::next(it).base());
			if (linkedIdx.has_value()) {
				map_->objects.erase(map_->objects.begin() +
						    static_cast<std::ptrdiff_t>(
							    *linkedIdx));
			}
			notifyMapEdited();
			return;
		}
	}
	for (auto it = map_->objects.rbegin(); it != map_->objects.rend(); ++it) {
		const bool erasable =
			it->role == worldmodel::ObjectRole::Decoration ||
			it->role == worldmodel::ObjectRole::Building ||
			map_->id == editor::kHomeMapId || map_->id == "town";
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

void MapCanvasView::moveAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	if (!moveTarget_.has_value()) {
		moveTarget_ = pickMoveTarget(cell);
		layerItem_->setMoveTarget(moveTarget_);
		return;
	}
	const auto it = moveAppliers_.find(moveTarget_->kind);
	if (it != moveAppliers_.end()) {
		(this->*(it->second))(moveTarget_->index, cell);
	}
	clearMoveTarget();
	notifyMapEdited();
}

std::optional<MoveTarget> MapCanvasView::pickMoveTarget(
	worldmodel::TilePos cell) const {
	for (std::size_t i = map_->markers.size(); i-- > 0;) {
		const int height =
			rules_.markerSpec(map_->markers[i].kind).height;
		if (containsCell(map_->markers[i].cell, 1, height, cell)) {
			return MoveTarget{MoveKind::Marker, i};
		}
	}
	for (std::size_t i = map_->portals.size(); i-- > 0;) {
		const auto& p = map_->portals[i];
		if (containsCell(p.cell, p.width, p.height, cell)) {
			return MoveTarget{MoveKind::Portal, i};
		}
	}
	for (std::size_t i = map_->objects.size(); i-- > 0;) {
		const auto& o = map_->objects[i];
		if (containsCell(o.cell, o.width, o.height, cell)) {
			return MoveTarget{MoveKind::Object, i};
		}
	}
	return std::nullopt;
}

void MapCanvasView::moveObject(std::size_t index, worldmodel::TilePos cell) {
	if (index >= map_->objects.size()) {
		return;
	}
	auto& obj = map_->objects[index];
	const int dCol = cell.column - obj.cell.column;
	const int dRow = cell.row - obj.cell.row;
	obj.cell = cell;
	if (obj.door.has_value()) {
		obj.door->column += dCol;
		obj.door->row += dRow;
	}
}

void MapCanvasView::moveMarker(std::size_t index, worldmodel::TilePos cell) {
	if (index >= map_->markers.size()) {
		return;
	}
	if (!validateMarkerPlacement(map_->markers[index].kind, cell)) {
		return;
	}
	map_->markers[index].cell = cell;
}

void MapCanvasView::movePortal(std::size_t index, worldmodel::TilePos cell) {
	if (index >= map_->portals.size()) {
		return;
	}
	const auto linkedIdx = linkedPortalObjectIndex(map_->portals[index]);
	map_->portals[index].cell = cell;
	if (linkedIdx.has_value()) {
		map_->objects[*linkedIdx].cell = cell;
	}
}

void MapCanvasView::clearMoveTarget() {
	moveTarget_.reset();
	layerItem_->setMoveTarget(std::nullopt);
}

void MapCanvasView::placePortalAt(worldmodel::TilePos cell) {
	if (!map_) {
		return;
	}
	if (const worldmodel::PrefabDef* prefab =
		    portalPrefabs_.prefabFor(activePortalKind_, catalog_)) {
		map_->objects.push_back(makeObjectFromPrefab(*prefab, cell));
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
	if (!activePrefab_->door.has_value()) {
		return std::nullopt;
	}
	return worldmodel::TilePos{
		cell.column + activePrefab_->door->column,
		cell.row + activePrefab_->door->row};
}

std::optional<std::size_t> MapCanvasView::linkedPortalObjectIndex(
	const worldmodel::Portal& portal) const {
	const worldmodel::PrefabDef* prefab =
		portalPrefabs_.prefabFor(portal.kind, catalog_);
	if (prefab == nullptr) {
		return std::nullopt;
	}
	for (std::size_t i = map_->objects.size(); i-- > 0;) {
		const auto& obj = map_->objects[i];
		if (obj.role == worldmodel::ObjectRole::Decoration &&
		    obj.cell == portal.cell && obj.tile == prefab->tile) {
			return i;
		}
	}
	return std::nullopt;
}

worldmodel::MapObject MapCanvasView::makeObjectFromPrefab(
	const worldmodel::PrefabDef& prefab, worldmodel::TilePos cell,
	std::optional<worldmodel::TilePos> door, int doorWidth) const {
	const bool anyBlocked = std::any_of(prefab.collisionGrid.begin(),
					    prefab.collisionGrid.end(),
					    [](bool b) { return b; });
	return worldmodel::MapObject{
		prefab.role.value_or(worldmodel::ObjectRole::Decoration),
		prefab.tile,
		cell,
		prefab.width,
		prefab.height,
		anyBlocked,
		door,
		prefab.collisionGrid,
		doorWidth,
	};
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
