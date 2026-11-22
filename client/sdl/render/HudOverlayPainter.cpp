#include "sdl/render/HudOverlayPainter.h"

#include <string>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TextRenderer.h"
#include "world/HudFillLayout.h"

namespace sdl {

HudOverlayPainter::HudOverlayPainter(SdlContext& context, TextRenderer& text,
				     const world::HudOverlayCatalog& catalog)
	: context_(context)
	, text_(text)
	, catalog_(catalog)
	, anchors_()
	, sheets_(context)
	, veilFills_{
		  {world::BlendMode::Alpha, &HudOverlayPainter::fillAlpha},
		  {world::BlendMode::Multiply,
		   &HudOverlayPainter::fillMultiply},
	  } {}

void HudOverlayPainter::fillVeil(const world::HudOverlay& overlay,
				 const double intensity) {
	(this->*veilFills_.at(overlay.blend))(overlay, intensity);
}

void HudOverlayPainter::fillAlpha(const world::HudOverlay& overlay,
				  const double intensity) {
	const world::Color& color = *overlay.color;
	context_.fillRect(
		SDL2pp::Rect(0, 0, context_.width(), context_.height()),
		SDL2pp::Color(static_cast<Uint8>(color.r),
			      static_cast<Uint8>(color.g),
			      static_cast<Uint8>(color.b),
			      static_cast<Uint8>(
				      catalog_.alphaFor(overlay, intensity))));
}

void HudOverlayPainter::fillMultiply(const world::HudOverlay& overlay,
				     const double intensity) {
	const world::Color color =
		catalog_.multiplyColorFor(overlay, intensity);
	context_.fillRectMultiply(
		SDL2pp::Rect(0, 0, context_.width(), context_.height()),
		SDL2pp::Color(static_cast<Uint8>(color.r),
			      static_cast<Uint8>(color.g),
			      static_cast<Uint8>(color.b),
			      static_cast<Uint8>(color.a)));
}

void HudOverlayPainter::drawLabels(const std::map<std::string, int>& values) {
	const int windowWidth = context_.width();
	const int windowHeight = context_.height();

	for (const world::HudLabel& label : catalog_.labels()) {
		const world::PixelBox box = anchors_.box(
			label.anchor, label.marginX, label.marginY,
			label.boxWidth, label.boxHeight, windowWidth,
			windowHeight);
		text_.draw(catalog_.textFor(label, values), box, label.color,
			   label.fontSize, label.bold);
	}
}

void HudOverlayPainter::draw(const std::map<std::string, double>& fillValues,
			     const double intensity) {
	const int windowWidth = context_.width();
	const int windowHeight = context_.height();

	for (const world::HudOverlay& overlay : catalog_.overlays()) {
		const int alpha = catalog_.alphaFor(overlay, intensity);
		if (alpha <= 0) {
			continue;
		}
		if (overlay.color.has_value()) {
			// Velo de color plano: cubre toda la ventana.
			fillVeil(overlay, intensity);
			continue;
		}
		// Tamaño final en píxeles: la región (en celdas) escalada.
		const int width =
			overlay.region.cols * overlay.cellWidth * overlay.scale;
		const int height =
			overlay.region.rows * overlay.cellHeight *
			overlay.scale;

		const world::PixelBox box = anchors_.box(
			overlay.anchor, overlay.marginX, overlay.marginY, width,
			height, windowWidth, windowHeight);

		SpriteSheet& sheet = sheets_.sheet(
			overlay.sheetImage,
			core::assetPath(overlay.sheetImage), overlay.cellWidth,
			overlay.cellHeight);
		sheet.drawBlock(overlay.region.at.col, overlay.region.at.row,
				overlay.region.cols, overlay.region.rows,
				SDL2pp::Rect(box.x, box.y, box.width,
					     box.height));
		drawFill(overlay, box, fillValues);
	}
}

void HudOverlayPainter::drawFill(
	const world::HudOverlay& overlay, const world::PixelBox& shell,
	const std::map<std::string, double>& values) {
	if (!overlay.fill.has_value()) {
		return;
	}
	const world::HudFill& fill = *overlay.fill;
	double fraction = 0.0;
	if (const auto found = values.find(fill.value); found != values.end()) {
		fraction = found->second;
	}

	const world::HudFillLayout layout(shell, fill, overlay.scale);
	const world::PixelBox box = layout.box(fraction);
	if (box.height <= 0) {
		return;
	}

	SpriteSheet& sheet = sheets_.sheet(
		fill.sheetImage, core::assetPath(fill.sheetImage),
		fill.cellWidth, fill.cellHeight);
	sheet.drawContentStretched(fill.region.at.col, fill.region.at.row,
				   SDL2pp::Rect(box.x, box.y, box.width,
						box.height));
}

}
