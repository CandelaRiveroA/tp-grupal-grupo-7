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

MapLayerItem::MapLayerItem(const TilesetCache& tilesets) : tilesets_(tilesets) {}

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
	return map_ != nullptr && tool_ == EditorTool::Place &&
	       activePrefab_ != nullptr && hoverCell_.has_value() &&
	       map_->contains(*hoverCell_);
}

void MapLayerItem::drawPlacementPreview(QPainter* painter, int ts) const {
	const int x = hoverCell_->column * ts;
	const int y = hoverCell_->row * ts;
	const int w = activePrefab_->width * ts;
	const int h = activePrefab_->height * ts;
	const bool valid = map_->fits(*hoverCell_, activePrefab_->width,
				      activePrefab_->height);

	painter->save();
	painter->setClipRect(boundingRect());
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
	painter->restore();
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

void MapLayerItem::drawObjects(QPainter* painter, int ts, int minCol, int maxCol,
			       int minRow, int maxRow) const {
	for (const auto& obj : map_->objects) {
		if (obj.cell.column + obj.width - 1 < minCol ||
		    obj.cell.column > maxCol ||
		    obj.cell.row + obj.height - 1 < minRow ||
		    obj.cell.row > maxRow) {
			continue;
		}
		// Edificios sin sprite de prefab (placeholder de WorldFactory) se muestran como recuadro.
		if (obj.role != worldmodel::ObjectRole::Decoration &&
		    !obj.tile.tileset.starts_with("prefab_")) {
			painter->save();
			painter->setPen(QPen(QColor(255, 220, 80, 220), 0));
			painter->setBrush(QColor(255, 200, 50, 70));
			painter->drawRect(obj.cell.column * ts,
					  obj.cell.row * ts,
					  obj.width * ts,
					  obj.height * ts);
			painter->restore();
			continue;
		}
		painter->drawPixmap(
			obj.cell.column * ts, obj.cell.row * ts,
			tilesets_.tilePixmap(obj.tile, obj.width, obj.height));
	}
}

void MapLayerItem::drawMarkersAndPortals(QPainter* painter, int ts, int minCol,
					 int maxCol, int minRow,
					 int maxRow) const {
	const auto drawCells = [&](const auto& items, const QColor& border,
				   const QColor& fill) {
		painter->setPen(QPen(border, 0));
		painter->setBrush(fill);
		for (const auto& item : items) {
			if (item.cell.column >= minCol &&
			    item.cell.column <= maxCol &&
			    item.cell.row >= minRow && item.cell.row <= maxRow) {
				painter->drawRect(item.cell.column * ts,
						  item.cell.row * ts, ts, ts);
			}
		}
	};

	drawCells(map_->markers, QColor(0, 230, 255, 200),
		  QColor(0, 210, 230, 110));
	drawCells(map_->portals, QColor(190, 100, 255, 200),
		  QColor(170, 70, 230, 110));
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
