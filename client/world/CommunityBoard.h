#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/CreatureCatalog.h"
#include "world/CropQuality.h"

namespace world {

enum class CommunityVisualKind {
	None,
	ContentIcon,
	CropQualities,
	Creature,
	Coin,
};

// Fuente visual única y excluyente para una meta del Centro Comunitario.
struct CommunityGoalVisual {
	CommunityVisualKind kind = CommunityVisualKind::None;
	std::string iconId;
	std::vector<CropQuality> cropQualities;
	std::optional<CreatureKind> creature;
	int spriteSize = 0;
	int spriteOffsetY = 0;
};

// Una fila lista para dibujar del tablón del Centro Comunitario: nombre,
// cantidad formateada ("3 / 10", "0 / 0"), progreso numérico para la barra
// y su especificación visual.
struct CommunityRow {
	std::string label;
	std::string amount;
	bool done = false;
	CommunityGoalVisual visual;
	int current = 0;
	int total = 0;
};

// Un bundle (una página del tablón) con su texto de ayuda, si acepta
// donaciones y sus metas.
struct CommunitySection {
	std::string title;
	std::string helpText;
	bool done = false;
	bool donatable = true;
	std::vector<CommunityRow> rows;
};

}  // namespace world
