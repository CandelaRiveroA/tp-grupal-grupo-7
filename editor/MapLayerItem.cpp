#include "MapLayerItem.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <algorithm>
#include <cmath>

#include "Constants.h"
#include "TilesetCache.h"
#include "common/world_model/Map.h"
#include "common/world_model/MapObject.h"
#include "common/world_model/Marker.h"
#include "common/world_model/Portal.h"
#include "common/world_model/PrefabCatalog.h"

MapLayerItem::MapLayerItem(const TilesetCache& tilesets)
	: tilesets_(tilesets),
	  previewChecks_{
		  {EditorTool::Place, &MapLayerItem::hasPrefabPreview},
		  {EditorTool::Move, &MapLayerItem::hasMovePreview},
		  {EditorTool::PlacePortal, &MapLayerItem::alwaysPreview},
		  {EditorTool::PickTarget, &MapLayerItem::alwaysPreview},
	  },
	  previewDrawers_{
		  {EditorTool::Place, &MapLayerItem::drawPrefabPreview},
		  {EditorTool::Move, &MapLayerItem::drawMovePreview},
		  {EditorTool::PlacePortal,
		   &MapLayerItem::drawPlacePortalPreview},
		  {EditorTool::PickTarget,
		   &MapLayerItem::drawPickTargetPreview},
	  },
	  movePreviewDrawers_{
		  {MoveKind::Object, &MapLayerItem::drawMoveObjectPreview},
		  {MoveKind::Marker, &MapLayerItem::drawMoveMarkerPreview},
		  {MoveKind::Portal, &MapLayerItem::drawMovePortalPreview},
	  } {}

void MapLayerItem::setMap(const worldmodel::Map* map) {
	prepareGeometryChange();
	map_ = map;
	update();
}

void MapLayerItem::setShowCollision(bool show) {
	if (showCollision_ == show) {
		return;
	}
	showCollision_ = show;
	update();
}

void MapLayerItem::setTool(EditorTool tool) {
	if (tool_ == tool) {
		return;
	}
	const bool hadPreview = hasPreview();
	tool_ = tool;
	if (hadPreview || hasPreview()) {
		update();
	}
}

void MapLayerItem::setActivePrefab(const worldmodel::PrefabDef* prefab) {
	if (activePrefab_ == prefab) {
		return;
	}
	const bool hadPreview = hasPreview();
	activePrefab_ = prefab;
	if (hadPreview || hasPreview()) {
		update();
	}
}

void MapLayerItem::setMoveTarget(std::optional<MoveTarget> target) {
	const bool hadPreview = hasPreview();
	moveTarget_ = target;
	if (hadPreview || hasPreview()) {
		update();
	}
}

void MapLayerItem::setPortalPreviewSize(int width, int height) {
	portalPreviewW_ = std::max(1, width);
	portalPreviewH_ = std::max(1, height);
	if (hasPreview()) {
		update();
	}
}

void MapLayerItem::setHoverCell(std::optional<worldmodel::TilePos> cell) {
	if (hoverCell_ == cell) {
		return;
	}
	const bool hadPreview = hasPreview();
	hoverCell_ = cell;
	if (hadPreview || hasPreview()) {
		update();
	}
}

bool MapLayerItem::hasValidMap() const {
	return map_ != nullptr && map_->width > 0 && map_->height > 0 &&
	       map_->tileSize > 0;
}

QRectF MapLayerItem::boundingRect() const {
	if (!hasValidMap()) {
		return QRectF();
	}
	return QRectF(0, 0, map_->width * map_->tileSize,
		      map_->height * map_->tileSize);
}

void MapLayerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
			 QWidget*) {
	if (!hasValidMap()) {
		return;
	}

	const int ts = map_->tileSize;
	const QRectF exposed = option->exposedRect;
	const int minCol = std::max(0, static_cast<int>(std::floor(exposed.left() / ts)));
	const int maxCol = std::min(map_->width - 1,
				    static_cast<int>(std::floor(exposed.right() / ts)));
	const int minRow = std::max(0, static_cast<int>(std::floor(exposed.top() / ts)));
	const int maxRow = std::min(map_->height - 1,
				    static_cast<int>(std::floor(exposed.bottom() / ts)));
	if (minCol > maxCol || minRow > maxRow) {
		return;
	}

	drawGridLayer(painter, map_->ground, ts, minCol, maxCol, minRow, maxRow);
	drawGridLayer(painter, map_->paths, ts, minCol, maxCol, minRow, maxRow);
	drawObjects(painter, ts, minCol, maxCol, minRow, maxRow);
	drawMarkersAndPortals(painter, ts, minCol, maxCol, minRow, maxRow);
	drawCellGrid(painter, ts, minCol, maxCol, minRow, maxRow);

	if (showCollision_) {
		drawCollisionOverlay(painter, ts, minCol, maxCol, minRow, maxRow);
	}
	if (hasPreview()) {
		drawPlacementPreview(painter, ts);
	}
}

bool MapLayerItem::hasPreview() const {
	if (!map_ || !hoverCell_.has_value() || !map_->contains(*hoverCell_)) {
		return false;
	}
	const auto it = previewChecks_.find(tool_);
	return it != previewChecks_.end() && (this->*(it->second))();
}

bool MapLayerItem::hasPrefabPreview() const {
	return activePrefab_ != nullptr;
}

bool MapLayerItem::hasMovePreview() const {
	if (!moveTarget_.has_value()) {
		return false;
	}
	if (moveTarget_->kind == MoveKind::Object) {
		return moveTarget_->index < map_->objects.size();
	}
	if (moveTarget_->kind == MoveKind::Marker) {
		return moveTarget_->index < map_->markers.size();
	}
	return moveTarget_->index < map_->portals.size();
}

bool MapLayerItem::alwaysPreview() const {
	return true;
}

void MapLayerItem::drawPlacementPreview(QPainter* painter, int ts) const {
	const auto it = previewDrawers_.find(tool_);
	if (it == previewDrawers_.end()) {
		return;
	}
	painter->save();
	painter->setClipRect(boundingRect());
	(this->*(it->second))(painter, ts);
	painter->restore();
}

void MapLayerItem::drawPrefabPreview(QPainter* painter, int ts) const {
	const int x = hoverCell_->column * ts;
	const int y = hoverCell_->row * ts;
	const int w = activePrefab_->width * ts;
	const int h = activePrefab_->height * ts;
	const bool valid = map_->fits(*hoverCell_, activePrefab_->width,
				      activePrefab_->height);
	painter->setOpacity(editor::kGhostOpacity);
	painter->drawPixmap(
		x, y,
		tilesets_.tilePixmap(activePrefab_->tile, activePrefab_->width,
				     activePrefab_->height));
	painter->setOpacity(1.0);
	painter->setPen(QPen(
		valid ? QColor(40, 210, 80) : QColor(220, 50, 50), 0));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(x, y, w, h);
}

void MapLayerItem::drawMovePreview(QPainter* painter, int ts) const {
	const auto it = movePreviewDrawers_.find(moveTarget_->kind);
	if (it != movePreviewDrawers_.end()) {
		(this->*(it->second))(painter, ts);
	}
}

void MapLayerItem::drawMoveObjectPreview(QPainter* painter, int ts) const {
	const auto& obj = map_->objects[moveTarget_->index];
	const int x = hoverCell_->column * ts;
	const int y = hoverCell_->row * ts;
	painter->setOpacity(editor::kGhostOpacity);
	drawObjectAt(painter, obj, x, y, ts);
	painter->setOpacity(1.0);
	painter->setPen(QPen(QColor(255, 255, 255, 220), 0));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(x, y, obj.width * ts, obj.height * ts);
}

void MapLayerItem::drawMoveMarkerPreview(QPainter* painter, int ts) const {
	painter->setPen(QPen(QColor(0, 230, 255), 0));
	painter->setBrush(QColor(0, 210, 230, 140));
	painter->drawRect(hoverCell_->column * ts, hoverCell_->row * ts, ts,
			  ts);
}

void MapLayerItem::drawMovePortalPreview(QPainter* painter, int ts) const {
	const auto& portal = map_->portals[moveTarget_->index];
	painter->setPen(QPen(QColor(190, 100, 255), 0));
	painter->setBrush(QColor(170, 70, 230, 140));
	painter->drawRect(hoverCell_->column * ts, hoverCell_->row * ts,
			  portal.width * ts, portal.height * ts);
}

void MapLayerItem::drawPlacePortalPreview(QPainter* painter, int ts) const {
	painter->setPen(QPen(QColor(190, 100, 255), 0));
	painter->setBrush(QColor(170, 70, 230, 140));
	painter->drawRect(hoverCell_->column * ts, hoverCell_->row * ts,
			  portalPreviewW_ * ts, portalPreviewH_ * ts);
}

void MapLayerItem::drawPickTargetPreview(QPainter* painter, int ts) const {
	painter->setPen(QPen(QColor(40, 230, 110), 0));
	painter->setBrush(QColor(40, 230, 110, 90));
	painter->drawRect(hoverCell_->column * ts, hoverCell_->row * ts,
			  portalPreviewW_ * ts, portalPreviewH_ * ts);
}

void MapLayerItem::drawGridLayer(QPainter* painter, const worldmodel::TileGrid& grid,
				 int ts, int minCol, int maxCol, int minRow,
				 int maxRow) const {
	for (int row = minRow; row <= maxRow; ++row) {
		for (int col = minCol; col <= maxCol; ++col) {
			const std::size_t idx = map_->cellIndex({col, row});
			if (idx < grid.size() && grid[idx].has_value()) {
				painter->drawPixmap(col * ts, row * ts,
						    tilesets_.tilePixmap(*grid[idx]));
			}
		}
	}
}

void MapLayerItem::drawObjectAt(QPainter* painter,
				const worldmodel::MapObject& obj, int x, int y,
				int ts) const {
	// Edificios sin sprite de prefab (placeholder de WorldFactory) se muestran como recuadro.
	if (obj.role != worldmodel::ObjectRole::Decoration &&
	    !obj.tile.tileset.starts_with("prefab_")) {
		painter->save();
		painter->setPen(QPen(QColor(255, 220, 80, 220), 0));
		painter->setBrush(QColor(255, 200, 50, 70));
		painter->drawRect(x, y, obj.width * ts, obj.height * ts);
		painter->restore();
		return;
	}
	painter->drawPixmap(
		x, y, tilesets_.tilePixmap(obj.tile, obj.width, obj.height));
}

void MapLayerItem::drawObjects(QPainter* painter, int ts, int minCol, int maxCol,
			       int minRow, int maxRow) const {
	for (const auto& obj : map_->objects) {
		if (obj.cell.column + obj.width - 1 < minCol ||
		    obj.cell.column > maxCol ||
		    obj.cell.row + obj.height - 1 < minRow ||
		    obj.cell.row > maxRow) {
			continue;
		}
		drawObjectAt(painter, obj, obj.cell.column * ts,
			     obj.cell.row * ts, ts);
	}
}

void MapLayerItem::drawMarkersAndPortals(QPainter* painter, int ts, int minCol,
					 int maxCol, int minRow,
					 int maxRow) const {
	painter->setPen(QPen(QColor(0, 230, 255, 200), 0));
	painter->setBrush(QColor(0, 210, 230, 110));
	for (const auto& marker : map_->markers) {
		if (marker.cell.column >= minCol &&
		    marker.cell.column <= maxCol &&
		    marker.cell.row >= minRow && marker.cell.row <= maxRow) {
			painter->drawRect(marker.cell.column * ts,
					  marker.cell.row * ts, ts, ts);
		}
	}

	painter->setPen(QPen(QColor(190, 100, 255, 200), 0));
	painter->setBrush(QColor(170, 70, 230, 110));
	for (const auto& portal : map_->portals) {
		if (portal.cell.column + portal.width - 1 < minCol ||
		    portal.cell.column > maxCol ||
		    portal.cell.row + portal.height - 1 < minRow ||
		    portal.cell.row > maxRow) {
			continue;
		}
		painter->drawRect(portal.cell.column * ts,
				  portal.cell.row * ts, portal.width * ts,
				  portal.height * ts);
	}
	painter->setBrush(Qt::NoBrush);
}

void MapLayerItem::drawCellGrid(QPainter* painter, int ts, int minCol, int maxCol,
				int minRow, int maxRow) const {
	// Pen cosmetico (ancho 0) para mantener 1px limpio sin importar el zoom.
	painter->setPen(QPen(QColor(0, 0, 0, 60), 0));
	const int top = minRow * ts;
	const int bottom = (maxRow + 1) * ts;
	const int left = minCol * ts;
	const int right = (maxCol + 1) * ts;

	for (int col = minCol; col <= maxCol + 1; ++col) {
		const int x = col * ts;
		painter->drawLine(x, top, x, bottom);
	}
	for (int row = minRow; row <= maxRow + 1; ++row) {
		const int y = row * ts;
		painter->drawLine(left, y, right, y);
	}
}

void MapLayerItem::drawCollisionOverlay(QPainter* painter, int ts, int minCol,
					int maxCol, int minRow,
					int maxRow) const {
	for (int row = minRow; row <= maxRow; ++row) {
		for (int col = minCol; col <= maxCol; ++col) {
			const std::size_t idx = map_->cellIndex({col, row});
			const bool base = idx < map_->baseCollision.size() &&
					  map_->baseCollision[idx];
			const bool total = idx < map_->collision.size() &&
					   map_->collision[idx];
			if (base) {
				painter->fillRect(col * ts, row * ts, ts, ts,
						  QColor(220, 40, 40, 100));
			} else if (total) {
				painter->fillRect(col * ts, row * ts, ts, ts,
						  QColor(245, 150, 30, 120));
			}
		}
	}
}
