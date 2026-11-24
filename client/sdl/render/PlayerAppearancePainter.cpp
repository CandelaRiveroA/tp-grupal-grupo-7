#include "sdl/render/PlayerAppearancePainter.h"

#include <algorithm>
#include <cstddef>

#include "core/Constants.h"
#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterLayout.h"
#include "world/CharacterPalettes.h"

namespace sdl {

namespace {

// Colores testigo que trae el arte base de Stardew. El recolor de piel
// los cambia por la fila elegida de skinColors; las mangas se reemplazan
// por tres tonos muestreados de la camisa; las pupilas por el color de
// ojos. Son datos del arte, no reglas que el jugador pueda cambiar.
constexpr int kSkinTones = 3;
constexpr std::array<std::array<std::uint8_t, 3>, 3> kSleeveMarkers = {{
	{{74, 12, 6}},
	{{112, 23, 24}},
	{{142, 31, 12}},
}};
constexpr int kSleeveSampleX = 0;
// Tonos de la manga muestreados de la camisa, en el orden de los testigos:
// oscuro, medio y claro.
constexpr std::array<int, 3> kSleeveSampleY = {{0, 3, 1}};

constexpr std::array<world::AppearanceSide, 4> kSides = {
	world::AppearanceSide::Front,
	world::AppearanceSide::Back,
	world::AppearanceSide::Side,
	world::AppearanceSide::Side,
};

}  // namespace

PlayerAppearancePainter::PlayerAppearancePainter(
	SdlContext& context, SheetCache& sheets,
	const world::CharacterCatalog& characters,
	const world::CharacterPalettes& palettes)
	: sheets_(sheets)
	, characters_(characters)
	, palettes_(palettes)
	, anchorBase_{
	      {world::AppearanceAnchor::Body,
	       [](int, int) { return 0; }},
	      {world::AppearanceAnchor::Head,
	       [](int crown, int) { return crown; }},
	      {world::AppearanceAnchor::Feet,
	       [](int, int feet) { return feet; }}}
	, composer_(context) {}

void PlayerAppearancePainter::draw(const game::ViewEntity& entity,
				   const world::Pose& pose,
				   const world::ActionKind action,
				   const int frameIndex,
				   const SDL2pp::Rect& dest,
				   const Flip flip) const {
	if (pose.empty() || dest.w <= 0 || dest.h <= 0) {
		return;
	}

	const world::AppearanceData& appearance =
		characters_.appearanceFor(entity.kind);
	if (!appearance.present) {
		return;
	}

	const int cellWidth = characters_.sheetCellWidth(entity.kind);
	const int cellHeight = characters_.sheetCellHeight(entity.kind);
	const world::LayerFrame& base = pose.front();
	const int canvasWidth = base.cols * cellWidth;
	const int canvasHeight = base.rows * cellHeight;
	if (canvasWidth <= 0 || canvasHeight <= 0) {
		return;
	}
	composer_.reset(canvasWidth, canvasHeight);

	const world::AppearanceSide side =
		kSides[static_cast<std::size_t>(entity.facing)];
	const std::size_t sideIndex = static_cast<std::size_t>(side);

	SpriteSheet& baseSheet = sheets_.sheet(
		characters_.sheetFile(entity.kind),
		core::assetPath(characters_.sheetFile(entity.kind)),
		cellWidth, cellHeight);

	// Coronilla y pies del frame (bordes del contenido del cuerpo): anclas
	// para lo que va sobre la cabeza y sobre las piernas. Salen del
	// contentRect de las celdas (cacheado).
	const int crownY = baseSheet.contentTop(base.frame.col, base.frame.row,
						base.cols, base.rows);
	const int feetY = baseSheet.contentBottom(base.frame.col, base.frame.row,
						  base.cols, base.rows);

	PixelRules skinRules;
	readSkinRules(appearance.skin, entity.appearance.skin, skinRules);

	const world::AppearanceOption* shirtOption =
		optionAt(appearance.shirt.options, entity.appearance.shirt);
	const world::SheetRegion* shirtRegion = regionAt(shirtOption, side);
	PixelRules sleeveRules;
	if (shirtRegion != nullptr) {
		SpriteSheet& shirtSheet = layerSheet(appearance.shirt);
		readSleeveRules(shirtSheet, *shirtRegion, sleeveRules);
	}

	// Cuerpo: la piel se recolorea. Los brazos se dibujan MÁS ABAJO, ya
	// con la ropa puesta, para que un brazo levantado en una acción no
	// quede tapado por el parche fijo de la camisa.
	for (const world::LayerFrame& layer : pose) {
		if (layer.layer != world::Layer::Body) {
			continue;
		}
		const world::SheetRegion region(layer.frame, layer.cols,
						layer.rows);
		composer_.drawRegion(baseSheet, region, 0, 0, skinRules);
	}

	// Ojos: sólo las pupilas que ya trae el cuerpo, antes de que el pelo
	// o los accesorios las tapen.
	if (!palettes_.eyes.empty()) {
		const int index = std::clamp(static_cast<int>(entity.appearance.eyes),
					     0,
					     static_cast<int>(palettes_.eyes.size()) - 1);
		const std::array<int, 3>& witness = palettes_.eyes.front();
		const std::array<int, 3>& rgb = palettes_.eyes[index];
		PixelRules rules;
		rules.recolors.push_back(PixelComposer::Recolor{
			Rgba{static_cast<std::uint8_t>(witness[0]),
			     static_cast<std::uint8_t>(witness[1]),
			     static_cast<std::uint8_t>(witness[2]), 255},
			Rgba{static_cast<std::uint8_t>(rgb[0]),
			     static_cast<std::uint8_t>(rgb[1]),
			     static_cast<std::uint8_t>(rgb[2]), 255}});
		composer_.recolorCanvas(rules);
	}

	// Pantalón: la opción tiene tres cuadros por cara (idle + dos de
	// caminata). Caminar usa los de piernas; el resto queda en el 0.
	const world::AppearanceOption* pantsOption =
		optionAt(appearance.pants.options, entity.appearance.pants);
	if (pantsOption != nullptr) {
		const std::vector<world::SheetRegion>& frames =
			pantsOption->sides[static_cast<std::size_t>(side)];
		if (!frames.empty()) {
			int frame = 0;
			if (action == world::ActionKind::Walk ||
			    action == world::ActionKind::Run ||
			    action == world::ActionKind::CarryWalk) {
				frame = std::min(frameIndex + 1,
						 static_cast<int>(frames.size()) - 1);
			}
			PixelRules rules;
			if (!palettes_.pants.empty()) {
				const int index = std::clamp(
					static_cast<int>(
						entity.appearance.pantsColor),
					0,
					static_cast<int>(palettes_.pants.size()) - 1);
				const std::array<int, 3>& rgb =
					palettes_.pants[index];
				rules.tint = true;
				rules.tintColor =
					Rgba{static_cast<std::uint8_t>(rgb[0]),
					     static_cast<std::uint8_t>(rgb[1]),
					     static_cast<std::uint8_t>(rgb[2]), 255};
			}
			const std::array<int, 2> pantsPos =
				placedAt(appearance.pants, sideIndex, crownY, feetY);
			composer_.drawRegion(layerSheet(appearance.pants),
					     frames[frame], pantsPos[0],
					     pantsPos[1], rules);
		}
	}

	// Camisa: torso de 8x8 en su offset (las mangas se pintan en los
	// brazos, más abajo).
	if (shirtRegion != nullptr) {
		const std::array<int, 2> shirtPos =
			placedAt(appearance.shirt, sideIndex, crownY, feetY);
		composer_.drawRegion(layerSheet(appearance.shirt), *shirtRegion,
				     shirtPos[0], shirtPos[1], PixelRules{});
	}

	// Brazos: encima de la ropa (orden de Stardew). La piel se recolorea
	// y las mangas testigo se reemplazan por los tonos de la camisa, así
	// el brazo va al frente con su manga en vez de quedar detrás del
	// parche fijo de la camisa.
	for (const world::LayerFrame& layer : pose) {
		if (layer.layer != world::Layer::Arms) {
			continue;
		}
		const world::SheetRegion region(layer.frame, layer.cols,
						layer.rows);
		PixelRules armsRules = skinRules;
		armsRules.merge(sleeveRules);
		const int y = canvasHeight - layer.rows * cellHeight;
		composer_.drawRegion(baseSheet, region, 0, y, armsRules);
	}

	// Accesorio: la opción 0 no tiene caras; espalda tampoco.
	const world::AppearanceOption* accessoryOption =
		optionAt(appearance.accessory.options,
			 entity.appearance.accessory);
	const world::SheetRegion* accessoryRegion =
		regionAt(accessoryOption, side);
	if (accessoryRegion != nullptr) {
		const std::array<int, 2> accessoryPos =
			placedAt(appearance.accessory, sideIndex, crownY, feetY);
		composer_.drawRegion(layerSheet(appearance.accessory),
				     *accessoryRegion, accessoryPos[0],
				     accessoryPos[1], PixelRules{});
	}

	// Pelo: va arriba de todo y se tiñe con el color elegido.
	const world::AppearanceOption* hairOption =
		optionAt(appearance.hair.options, entity.appearance.hair);
	const world::SheetRegion* hairRegion = regionAt(hairOption, side);
	if (hairRegion != nullptr) {
		PixelRules rules;
		if (!palettes_.hair.empty()) {
			const int index = std::clamp(
				static_cast<int>(entity.appearance.hairColor), 0,
				static_cast<int>(palettes_.hair.size()) - 1);
			const std::array<int, 3>& rgb = palettes_.hair[index];
			rules.tint = true;
			rules.tintColor =
				Rgba{static_cast<std::uint8_t>(rgb[0]),
				     static_cast<std::uint8_t>(rgb[1]),
				     static_cast<std::uint8_t>(rgb[2]), 255};
		}
		const std::array<int, 2> hairPos =
			placedAt(appearance.hair, sideIndex, crownY, feetY);
		composer_.drawRegion(layerSheet(appearance.hair), *hairRegion,
				     hairPos[0], hairPos[1], rules);
	}

	// El lienzo es el cuerpo nativo (1x2 celdas = 16x32), no el box de la
	// celda (cuadrado). Se dibuja en el rect del CUERPO que calcula
	// CharacterLayout —centrado y anclado abajo— para conservar la
	// proporción y el tamaño con el que se dibujaba antes (drawn 32x64).
	const world::CharacterLayout layout(
		world::PixelRect{dest.x, dest.y, dest.w, dest.h},
		characters_.drawnWidth(entity.kind),
		characters_.drawnHeight(entity.kind));
	const world::PixelRect body = layout.layerRect(base, base);

	composer_.present(
		SDL2pp::Rect(body.x, body.y, body.width, body.height), flip);
}

const world::AppearanceOption* PlayerAppearancePainter::optionAt(
	const std::vector<world::AppearanceOption>& options,
	const int index) const {
	if (options.empty()) {
		return nullptr;
	}
	const int clamped =
		std::clamp(index, 0, static_cast<int>(options.size()) - 1);
	return &options[static_cast<std::size_t>(clamped)];
}

const world::SheetRegion* PlayerAppearancePainter::regionAt(
	const world::AppearanceOption* option,
	const world::AppearanceSide side) const {
	if (option == nullptr) {
		return nullptr;
	}
	const std::vector<world::SheetRegion>& regions =
		option->sides[static_cast<std::size_t>(side)];
	if (regions.empty()) {
		return nullptr;
	}
	return &regions.front();
}

SpriteSheet& PlayerAppearancePainter::layerSheet(
	const world::AppearanceLayerData& layer) const {
	const std::string key = "appearance:" + layer.sheetImage;
	return sheets_.sheet(key, core::assetPath(layer.sheetImage),
			     layer.cellWidth, layer.cellHeight);
}

std::array<int, 2> PlayerAppearancePainter::placedAt(
	const world::AppearanceLayerData& layer, const std::size_t sideIndex,
	const int crownY, const int feetY) const {
	const std::array<int, 2>& offset = layer.offset[sideIndex];
	const int baseY = anchorBase_.at(layer.anchor)(crownY, feetY);
	return {offset[0], offset[1] + baseY};
}

void PlayerAppearancePainter::readSkinRules(
	const world::SkinAppearanceData& skin, const int index,
	PixelRules& rules) const {
	if (skin.count <= 0 || skin.paletteSheet.empty()) {
		return;
	}
	SpriteSheet& sheet = sheets_.sheet(
		"appearance:" + skin.paletteSheet,
		core::assetPath(skin.paletteSheet), skin.cellWidth,
		skin.cellHeight);
	const int row = std::clamp(index, 0, skin.count - 1);
	for (int i = 0; i < kSkinTones; ++i) {
		rules.recolors.push_back(PixelComposer::Recolor{
			composer_.sample(sheet, i, 0),
			composer_.sample(sheet, i, row)});
	}
}

void PlayerAppearancePainter::readSleeveRules(
	SpriteSheet& shirtSheet, const world::SheetRegion& region,
	PixelRules& rules) const {
	const int originX = region.at.col * shirtSheet.frameWidth();
	const int originY = region.at.row * shirtSheet.frameHeight();
	for (std::size_t i = 0; i < kSleeveMarkers.size(); ++i) {
		rules.recolors.push_back(PixelComposer::Recolor{
			Rgba{kSleeveMarkers[i][0], kSleeveMarkers[i][1],
			     kSleeveMarkers[i][2], 255},
			composer_.sample(shirtSheet, originX + kSleeveSampleX,
					 originY + kSleeveSampleY[i])});
	}
}

}  // namespace sdl
