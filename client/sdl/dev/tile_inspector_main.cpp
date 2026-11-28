
#include <SDL2/SDL.h>

#include <SDL2pp/SDL2pp.hh>

#include <algorithm>
#include <cstdio>
#include <string>

#include "core/Constants.h"
#include "core/WindowConfig.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/SpriteSheet.h"

namespace {

constexpr const char* kSheetRelPath = "Farm/Coop.png";
constexpr int kCellSize = 16;

const SDL2pp::Color kBackground(30, 30, 40, 255);
const SDL2pp::Color kGridLine(80, 80, 100, 255);
const SDL2pp::Color kGridLineStrong(140, 140, 170, 255);
const SDL2pp::Color kSelected(255, 220, 0, 255);
const SDL2pp::Color kPanelBorder(200, 200, 200, 255);
const SDL2pp::Color kScrollTrack(50, 50, 65, 255);
const SDL2pp::Color kScrollThumb(160, 160, 200, 255);

constexpr int kInitialSheetScale = 3;
constexpr int kMinSheetScale = 1;
constexpr int kMaxSheetScale = 12;

constexpr int kDetailScale = 16;

constexpr int kSheetMargin = 16;

constexpr int kPanStep = 48;

constexpr int kScrollBarSize = 8;

void drawGrid(sdl::SdlContext& ctx, int originX, int originY, int cols,
	      int rows, int step) {
	for (int c = 0; c <= cols; ++c) {
		const SDL2pp::Color& color =
			(c % 5 == 0) ? kGridLineStrong : kGridLine;
		const int x = originX + c * step;
		ctx.fillRect(SDL2pp::Rect(x, originY, 1, rows * step), color);
	}
	for (int r = 0; r <= rows; ++r) {
		const SDL2pp::Color& color =
			(r % 5 == 0) ? kGridLineStrong : kGridLine;
		const int y = originY + r * step;
		ctx.fillRect(SDL2pp::Rect(originX, y, cols * step, 1), color);
	}
}

void drawCellOutline(sdl::SdlContext& ctx, int originX, int originY,
		     int col, int row, int step, int thickness,
		     const SDL2pp::Color& color) {
	const int x = originX + col * step;
	const int y = originY + row * step;
	ctx.fillRect(SDL2pp::Rect(x, y, step, thickness), color);
	ctx.fillRect(SDL2pp::Rect(x, y + step - thickness, step, thickness),
		     color);
	ctx.fillRect(SDL2pp::Rect(x, y, thickness, step), color);
	ctx.fillRect(SDL2pp::Rect(x + step - thickness, y, thickness, step),
		     color);
}

void drawScrollBar(sdl::SdlContext& ctx, bool horizontal, int content,
		   int view, int off, int windowW, int windowH) {
	if (content <= view) {
		return;
	}
	const int trackLen = (horizontal ? windowW : windowH) - 2 * kSheetMargin;
	const double ratio = static_cast<double>(view) / content;
	const int thumbLen = std::max(16, static_cast<int>(trackLen * ratio));
	const double scrolled =
		static_cast<double>(off) / (content - view);
	const int thumbPos = static_cast<int>(
		(trackLen - thumbLen) * std::clamp(scrolled, 0.0, 1.0));

	if (horizontal) {
		const int y = windowH - kScrollBarSize - 2;
		ctx.fillRect(SDL2pp::Rect(kSheetMargin, y, trackLen,
					  kScrollBarSize),
			     kScrollTrack);
		ctx.fillRect(SDL2pp::Rect(kSheetMargin + thumbPos, y, thumbLen,
					  kScrollBarSize),
			     kScrollThumb);
	} else {
		const int x = windowW - kScrollBarSize - 2;
		ctx.fillRect(SDL2pp::Rect(x, kSheetMargin, kScrollBarSize,
					  trackLen),
			     kScrollTrack);
		ctx.fillRect(SDL2pp::Rect(x, kSheetMargin + thumbPos,
					  kScrollBarSize, thumbLen),
			     kScrollThumb);
	}
}

int clampIndex(int value, int max) {
	if (value < 0) {
		return 0;
	}
	if (value >= max) {
		return max - 1;
	}
	return value;
}

}

int main() {
	const core::WindowConfig config(core::kWindowWidth,
					core::kWindowHeight,
					"Tile Inspector (provisorio) — "
					"Character_Base.png 16x16");
	sdl::SdlContext context(config);

	const std::string sheetPath = core::assetPath(kSheetRelPath);

	sdl::SpriteSheet sheet(context, sheetPath, kCellSize, kCellSize);

	SDL2pp::Texture probe = context.createTexture(sheetPath);
	const int sheetW = probe.GetWidth();
	const int sheetH = probe.GetHeight();
	// Redondeo hacia arriba: si la hoja no es múltiplo exacto de la celda,
	// la última columna/fila parcial igual se cubre con grilla (si no, los
	// sprites del borde quedan fuera y no se pueden seleccionar).
	const int cols = (sheetW + kCellSize - 1) / kCellSize;
	const int rows = (sheetH + kCellSize - 1) / kCellSize;

	std::printf(
		"[tile_inspector] hoja: %s  (%dx%d px) -> %d columnas x %d "
		"filas de %dx%d\n",
		sheetPath.c_str(), sheetW, sheetH, cols, rows, kCellSize,
		kCellSize);
	std::printf(
		"[tile_inspector] flechas=mover(la vista sigue)  WASD=scroll  "
		"shift+flechas=+5  ESPACIO=imprimir  +/-=zoom  esc=salir\n");

	int selCol = 0;
	int selRow = 0;
	int sheetScale = kInitialSheetScale;
	int panX = 0;
	int panY = 0;

	auto reportSelection = [&]() {
		const SDL2pp::Rect r = sheet.frameRect(selCol, selRow);
		std::printf("col=%d row=%d -> x=%d y=%d w=%d h=%d\n", selCol,
			    selRow, r.x, r.y, r.w, r.h);
		std::fflush(stdout);
	};

	bool running = true;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
			if (event.type != SDL_KEYDOWN) {
				continue;
			}

			const bool shift =
				(SDL_GetModState() & KMOD_SHIFT) != 0;
			const int stepCells = shift ? 5 : 1;

			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				running = false;
				break;
			case SDLK_LEFT:
				selCol = clampIndex(selCol - stepCells, cols);
				break;
			case SDLK_RIGHT:
				selCol = clampIndex(selCol + stepCells, cols);
				break;
			case SDLK_UP:
				selRow = clampIndex(selRow - stepCells, rows);
				break;
			case SDLK_DOWN:
				selRow = clampIndex(selRow + stepCells, rows);
				break;
			case SDLK_a:
				panX -= kPanStep;
				break;
			case SDLK_d:
				panX += kPanStep;
				break;
			case SDLK_w:
				panY -= kPanStep;
				break;
			case SDLK_s:
				panY += kPanStep;
				break;
			case SDLK_PLUS:
			case SDLK_EQUALS:
			case SDLK_KP_PLUS:
				if (sheetScale < kMaxSheetScale) {
					++sheetScale;
				}
				break;
			case SDLK_MINUS:
			case SDLK_KP_MINUS:
				if (sheetScale > kMinSheetScale) {
					--sheetScale;
				}
				break;
			case SDLK_SPACE:
				reportSelection();
				break;
			default:
				break;
			}
		}

		if (!running) {
			break;
		}

		const int step = kCellSize * sheetScale;
		const int drawnW = sheetW * sheetScale;
		const int drawnH = sheetH * sheetScale;

		const int viewW = context.width() - 2 * kSheetMargin;
		const int viewH = context.height() - 2 * kSheetMargin;

		const int selX = selCol * step;
		const int selY = selRow * step;
		if (selX < panX) {
			panX = selX;
		} else if (selX + step > panX + viewW) {
			panX = selX + step - viewW;
		}
		if (selY < panY) {
			panY = selY;
		} else if (selY + step > panY + viewH) {
			panY = selY + step - viewH;
		}

		const int maxPanX = drawnW > viewW ? drawnW - viewW : 0;
		const int maxPanY = drawnH > viewH ? drawnH - viewH : 0;
		panX = std::clamp(panX, 0, maxPanX);
		panY = std::clamp(panY, 0, maxPanY);

		context.clear(kBackground);

		const int originX = kSheetMargin - panX;
		const int originY = kSheetMargin - panY;

		const SDL2pp::Rect sheetDest(originX, originY, drawnW, drawnH);
		context.drawTexture(probe, SDL2pp::Rect(0, 0, sheetW, sheetH),
				    sheetDest);

		drawGrid(context, originX, originY, cols, rows, step);
		drawCellOutline(context, originX, originY, selCol, selRow, step,
				2, kSelected);

		drawScrollBar(context, true, drawnW, viewW, panX,
			      context.width(), context.height());
		drawScrollBar(context, false, drawnH, viewH, panY,
			      context.width(), context.height());

		const int detailSize = kCellSize * kDetailScale;
		const int panelX =
			context.width() - detailSize - kSheetMargin -
			kScrollBarSize - 4;
		const int panelY = kSheetMargin;
		const SDL2pp::Rect detailDest(panelX, panelY, detailSize,
					      detailSize);
		sheet.draw(selCol, selRow, detailDest);
		drawCellOutline(context, panelX, panelY, 0, 0, detailSize, 2,
				kPanelBorder);

		context.present();

		SDL_Delay(16);
	}

	std::printf("[tile_inspector] saliendo.\n");
	return 0;
}
