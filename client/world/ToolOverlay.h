#pragma once

#include <map>
#include <string>
#include <vector>

#include "world/ViewDirection.h"
#include "world/CharacterAppearance.h"
#include "world/ItemCatalog.h"

namespace world {

// Anclaje vertical del frame del overlay respecto al cuerpo: arriba de la
// cabeza, a nivel del cuerpo o a los pies.
enum class FrameAnchor {
	Top,
	Body,
	Bottom,
};

// Un frame del overlay con sus ajustes por frame: anclaje, offset en px
// (x hacia las manos, y respecto al anclaje), ángulo de rotación (horario),
// pivote de giro en fracción del sprite (puede exceder 1 para agrandar el
// radio), espejado y escala.
struct ToolOverlayFrame {
	std::vector<LayerFrame> layers;
	FrameAnchor anchor = FrameAnchor::Top;
	int offsetX = 0;
	int offsetY = 0;
	double angle = 0.0;
	double pivotX = 0.5;
	double pivotY = 0.5;
	bool flip = false;
	double scale = 1.0;
};

struct ToolOverlayAction {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;
	int frameTicks = 0;
	std::vector<ToolOverlayFrame> frames;
};

using ToolOverlayAppearance = std::map<Direction, ToolOverlayAction>;

struct ToolOverlayCatalog {
	// Ajuste vertical (px) por anclaje: cada uno se afina por separado
	// porque el gap transparente de la hoja difiere según dónde se apoya.
	std::map<FrameAnchor, int> anchorOffsets;

	std::map<ItemKind, ToolOverlayAppearance> tools;

	bool has(ItemKind kind, Direction dir) const;

	const ToolOverlayAction& actionFor(ItemKind kind, Direction dir) const;

	int offsetFor(FrameAnchor anchor) const;
};

}
