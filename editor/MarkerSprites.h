#pragma once

#include <QPixmap>
#include <map>

#include "common/world_model/Marker.h"

// Sprites de los marcadores visibles en paleta y lienzo del editor.
class MarkerSprites {
public:
	MarkerSprites();

	const QPixmap& sprite(worldmodel::MarkerKind kind) const;

private:
	QPixmap defaultSprite_;
	std::map<worldmodel::MarkerKind, QPixmap> sprites_;
};
