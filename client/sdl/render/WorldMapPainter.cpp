#include "sdl/render/WorldMapPainter.h"

#include <filesystem>
#include <utility>

#include "sdl/context/SdlContext.h"
#include "sdl/render/PlayerAppearancePainter.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

WorldMapPainter::WorldMapPainter(
	SdlContext& context, InventoryHud& inventoryHud, TextRenderer& text,
	const PlayerAppearancePainter& appearancePainter,
	world::WorldMapConfig config, const std::string& minimapDir)
	: text_(text)
	, appearancePainter_(appearancePainter)
	, config_(std::move(config))
	, modalPainter_(context, inventoryHud, text, config_.modal) {
	for (const world::WorldMapExteriorConfig& ext : config_.exteriors) {
		std::optional<SDL2pp::Texture> texture;
		if (!minimapDir.empty()) {
			texture = context.tryLoadTexture(
				(std::filesystem::path(minimapDir) /
				 (ext.id + ".png"))
					.string());
		}
		exteriors_.emplace(
			ext.id, ExteriorEntry{ext.title, std::move(texture)});
	}
}

bool WorldMapPainter::hitsSwitchArrow(const int screenW, const int screenH,
				      const int x, const int y) const {
	return world::WorldMapLayout(screenW, screenH, config_,
				     world::PixelSize{0, 0})
		.hitsSwitchArrow(x, y);
}

world::WorldMapLayout WorldMapPainter::layoutFor(
	const int screenW, const int screenH,
	const std::string& exterior) const {
	world::PixelSize mapSize{0, 0};
	const auto it = exteriors_.find(exterior);
	if (it != exteriors_.end() && it->second.texture.has_value()) {
		mapSize = world::PixelSize{it->second.texture->GetWidth(),
					   it->second.texture->GetHeight()};
	}
	return world::WorldMapLayout(screenW, screenH, config_, mapSize);
}

void WorldMapPainter::draw(const std::string& activeExterior,
			   const std::vector<game::WorldMapPin>& pins,
			   const int screenW, const int screenH) {
	const world::WorldMapLayout layout =
		layoutFor(screenW, screenH, activeExterior);

	modalPainter_.drawShell(layout.modal(), screenW, screenH);

	const auto it = exteriors_.find(activeExterior);
	const std::string title =
		(it != exteriors_.end()) ? it->second.title : activeExterior;
	text_.draw(title, layout.titleBox(), config_.modal.inkColor,
		   config_.titleFontSize, true);

	if (it != exteriors_.end() && it->second.texture.has_value()) {
		SDL2pp::Texture& tex = *it->second.texture;
		modalPainter_.fillRect(layout.mapBorderBox(),
				       config_.modal.woodBorderColor);
		modalPainter_.drawTexture(tex, layout.mapRect());
	} else {
		text_.draw(config_.unavailableText, layout.mapRect(),
			   config_.modal.inkColor, config_.unavailableFontSize);
	}

	modalPainter_.drawArrows(layout.modal());

	// Dibujar primero compañeros y al final al jugador local para que su
	// aro dorado quede visible incluso compartiendo puerta o portal.
	for (const game::WorldMapPin& pin : pins) {
		if (!pin.isLocal) {
			drawPin(pin, layout);
		}
	}
	for (const game::WorldMapPin& pin : pins) {
		if (pin.isLocal) {
			drawPin(pin, layout);
		}
	}
}

void WorldMapPainter::drawPin(const game::WorldMapPin& pin,
			      const world::WorldMapLayout& layout) const {
	const world::PixelBox outerBox =
		layout.pinOuterBox(pin.normX, pin.normY);
	const world::PixelBox innerBox = layout.pinInnerBox(outerBox);

	const world::Color& ringColor =
		pin.isLocal ? config_.localRingColor
			    : config_.modal.woodBorderColor;
	modalPainter_.fillRect(outerBox, ringColor);
	modalPainter_.fillRect(innerBox, config_.modal.parchmentColor);
	appearancePainter_.drawHead(pin.appearance,
				    SDL2pp::Rect(innerBox.x, innerBox.y,
						 innerBox.width,
						 innerBox.height));
}

}  // namespace sdl
