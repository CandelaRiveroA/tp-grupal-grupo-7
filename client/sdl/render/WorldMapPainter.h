#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game/WorldMapState.h"
#include "sdl/render/PagedModalPainter.h"
#include "world/PixelBox.h"
#include "world/WorldMapConfig.h"
#include "world/WorldMapLayout.h"

namespace sdl {

class InventoryHud;
class PlayerAppearancePainter;
class SdlContext;
class TextRenderer;

// Dibuja el overlay modal del mapa del mundo reutilizando PagedModalPainter
// para el fondo, pergamino, marco y flechas.
class WorldMapPainter {
public:
	WorldMapPainter(SdlContext& context, InventoryHud& inventoryHud,
			TextRenderer& text,
			const PlayerAppearancePainter& appearancePainter,
			world::WorldMapConfig config,
			const std::string& minimapDir);

	bool hitsSwitchArrow(int screenW, int screenH, int x, int y) const;

	void draw(const std::string& activeExterior,
		  const std::vector<game::WorldMapPin>& pins, int screenW,
		  int screenH);

private:
	struct ExteriorEntry {
		std::string title;
		std::optional<SDL2pp::Texture> texture;
	};

	world::WorldMapLayout layoutFor(int screenW, int screenH,
					const std::string& exterior) const;
	void drawPin(const game::WorldMapPin& pin,
		     const world::WorldMapLayout& layout) const;

	TextRenderer& text_;
	const PlayerAppearancePainter& appearancePainter_;
	world::WorldMapConfig config_;
	PagedModalPainter modalPainter_;

	std::unordered_map<std::string, ExteriorEntry> exteriors_;
};

}  // namespace sdl
