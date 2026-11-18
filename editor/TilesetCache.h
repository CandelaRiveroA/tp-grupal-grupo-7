#pragma once

#include <QPixmap>
#include <map>
#include <string>

#include "common/world_model/Tile.h"

// Cache de hojas de tilesets en QPixmap para recortar tiles y sprites
// multi-celda sin recargar las imagenes en cada redibujo.
class TilesetCache {
public:
	explicit TilesetCache(std::string assetsDir);

	// Carga o actualiza las hojas PNG de los tilesets indicados.
	void loadTilesets(
			const std::map<std::string, worldmodel::TilesetDef>& tilesets);

	// Recorta un sprite de widthCells x heightCells celdas; si falta la
	// hoja o el rectangulo cae fuera, devuelve una grilla magenta/negra.
	QPixmap tilePixmap(const worldmodel::TileRef& ref, int widthCells = 1,
			   int heightCells = 1) const;

private:
	struct Entry {
		worldmodel::TilesetDef def;
		QPixmap sheet;
	};

	static QPixmap makeFallback(int width, int height);

	std::string assetsDir_;
	std::map<std::string, Entry> entries_;
};
