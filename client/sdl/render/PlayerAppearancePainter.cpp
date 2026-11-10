#include "sdl/render/PlayerAppearancePainter.h"

#include <algorithm>
#include <cstddef>

#include "core/Constants.h"
#include "game/LocalWorldView.h"
#include "sdl/context/SdlContext.h"
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
constexpr std::array<std::array<std::uint8_t, 3>, 3> kSleeveMarkers = {{
	{{74, 12, 6}},
	{{112, 23, 24}},
	{{142, 31, 12}},
}};
constexpr int kSleeveSampleX = 0;
constexpr int kSleeveDarkY = 0;
constexpr int kSleeveMidY = 3;
constexpr int kSleeveLightY = 1;

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
	: context_(context)
	, sheets_(sheets)
	, characters_(characters)
	, palettes_(palettes)
	, anchorBase_{
	      {world::AppearanceAnchor::Body,
	       [](int, int) { return 0; }},
	      {world::AppearanceAnchor::Head,
	       [](int crown, int) { return crown; }},
	      {world::AppearanceAnchor::Feet,
	       [](int, int feet) { return feet; }}} {}

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
	clearCanvas(canvasWidth, canvasHeight);

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
		blit(baseSheet, region, 0, 0, skinRules);
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
		rules.pupilRecolor = true;
		rules.pupilFrom =
			Rgba{static_cast<std::uint8_t>(witness[0]),
			     static_cast<std::uint8_t>(witness[1]),
			     static_cast<std::uint8_t>(witness[2]), 255};
		rules.pupilTo = Rgba{static_cast<std::uint8_t>(rgb[0]),
				     static_cast<std::uint8_t>(rgb[1]),
				     static_cast<std::uint8_t>(rgb[2]), 255};
		recolorCanvas(rules);
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
			blit(layerSheet(appearance.pants), frames[frame],
			     pantsPos[0], pantsPos[1], rules);
		}
	}

	// Camisa: torso de 8x8 en su offset (las mangas se pintan en los
	// brazos, más abajo).
	if (shirtRegion != nullptr) {
		const std::array<int, 2> shirtPos =
			placedAt(appearance.shirt, sideIndex, crownY, feetY);
		blit(layerSheet(appearance.shirt), *shirtRegion,
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
		armsRules.sleeveReplace = sleeveRules.sleeveReplace;
		armsRules.sleeveMarker = sleeveRules.sleeveMarker;
		armsRules.sleeveTone = sleeveRules.sleeveTone;
		const int y = canvasHeight - layer.rows * cellHeight;
		blit(baseSheet, region, 0, y, armsRules);
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
		blit(layerSheet(appearance.accessory), *accessoryRegion,
		     accessoryPos[0], accessoryPos[1], PixelRules{});
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
		blit(layerSheet(appearance.hair), *hairRegion,
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

	texture_->Update(SDL2pp::NullOpt, canvas_.data(),
			 canvasWidth_ * static_cast<int>(sizeof(std::uint32_t)));
	context_.drawTexture(
		*texture_,
		SDL2pp::Rect(0, 0, canvasWidth_, canvasHeight_),
		SDL2pp::Rect(body.x, body.y, body.width, body.height), flip);
}

void PlayerAppearancePainter::clearCanvas(const int width,
					  const int height) const {
	canvasWidth_ = width;
	canvasHeight_ = height;
	canvas_.assign(static_cast<std::size_t>(width) *
			       static_cast<std::size_t>(height),
		       0);
	if (!texture_.has_value() || textureWidth_ != width ||
	    textureHeight_ != height) {
		texture_.emplace(context_.createStreamingTexture(
			width, height, SDL_PIXELFORMAT_ARGB8888));
		texture_->SetBlendMode(SDL_BLENDMODE_BLEND);
		textureWidth_ = width;
		textureHeight_ = height;
	}
}

void PlayerAppearancePainter::blit(SpriteSheet& sheet,
				   const world::SheetRegion& region,
				   const int offsetX, const int offsetY,
				   const PixelRules& rules) const {
	if (region.cols <= 0 || region.rows <= 0) {
		return;
	}
	SDL2pp::Surface& surface = sheet.surface();
	SDL2pp::Surface::LockHandle lock = surface.Lock();
	const SDL_PixelFormat& format = lock.GetFormat();
	if (format.BytesPerPixel != 4) {
		return;
	}

	const int frameWidth = sheet.frameWidth();
	const int frameHeight = sheet.frameHeight();
	const int srcX = region.at.col * frameWidth;
	const int srcY = region.at.row * frameHeight;
	const int width = region.cols * frameWidth;
	const int height = region.rows * frameHeight;
	const int surfaceWidth = surface.GetWidth();
	const int surfaceHeight = surface.GetHeight();
	const auto* pixels =
		static_cast<const std::uint32_t*>(lock.GetPixels());
	const int pitch =
		lock.GetPitch() / static_cast<int>(sizeof(std::uint32_t));

	for (int y = 0; y < height; ++y) {
		const int sourceY = srcY + y;
		if (sourceY < 0 || sourceY >= surfaceHeight) {
			continue;
		}
		for (int x = 0; x < width; ++x) {
			const int sourceX = srcX + x;
			if (sourceX < 0 || sourceX >= surfaceWidth) {
				continue;
			}
			Rgba pixel;
			SDL_GetRGBA(pixels[sourceY * pitch + sourceX], &format,
				    &pixel.r, &pixel.g, &pixel.b, &pixel.a);
			blendPixel(offsetX + x, offsetY + y,
				   applyRules(pixel, rules));
		}
	}
}

void PlayerAppearancePainter::recolorCanvas(const PixelRules& rules) const {
	for (std::uint32_t& pixel : canvas_) {
		Rgba color = unpack(pixel);
		if (color.a != 255 || !rules.pupilRecolor) {
			continue;
		}
		if (color.r == rules.pupilFrom.r && color.g == rules.pupilFrom.g &&
		    color.b == rules.pupilFrom.b) {
			color.r = rules.pupilTo.r;
			color.g = rules.pupilTo.g;
			color.b = rules.pupilTo.b;
			pixel = pack(color);
		}
	}
}

PlayerAppearancePainter::Rgba PlayerAppearancePainter::sample(
	SpriteSheet& sheet, const int x, const int y) const {
	SDL2pp::Surface& surface = sheet.surface();
	if (x < 0 || y < 0 || x >= surface.GetWidth() ||
	    y >= surface.GetHeight()) {
		return Rgba{};
	}
	SDL2pp::Surface::LockHandle lock = surface.Lock();
	const SDL_PixelFormat& format = lock.GetFormat();
	if (format.BytesPerPixel != 4) {
		return Rgba{};
	}
	const auto* pixels =
		static_cast<const std::uint32_t*>(lock.GetPixels());
	const int pitch =
		lock.GetPitch() / static_cast<int>(sizeof(std::uint32_t));
	Rgba color;
	SDL_GetRGBA(pixels[y * pitch + x], &format, &color.r, &color.g,
		    &color.b, &color.a);
	return color;
}

PlayerAppearancePainter::Rgba PlayerAppearancePainter::applyRules(
	Rgba pixel, const PixelRules& rules) const {
	if (pixel.a == 255) {
		if (rules.skinRecolor) {
			for (std::size_t i = 0; i < rules.skinWitness.size();
			     ++i) {
				const Rgba& witness = rules.skinWitness[i];
				if (pixel.r == witness.r &&
				    pixel.g == witness.g &&
				    pixel.b == witness.b) {
					pixel.r = rules.skinTarget[i].r;
					pixel.g = rules.skinTarget[i].g;
					pixel.b = rules.skinTarget[i].b;
					break;
				}
			}
		}
		if (rules.sleeveReplace) {
			for (std::size_t i = 0; i < rules.sleeveMarker.size();
			     ++i) {
				const Rgba& marker = rules.sleeveMarker[i];
				if (pixel.r == marker.r &&
				    pixel.g == marker.g &&
				    pixel.b == marker.b) {
					const Rgba& tone = rules.sleeveTone[i];
					if (tone.a == 255) {
						pixel.r = tone.r;
						pixel.g = tone.g;
						pixel.b = tone.b;
					}
					break;
				}
			}
		}
		if (rules.pupilRecolor &&
		    pixel.r == rules.pupilFrom.r &&
		    pixel.g == rules.pupilFrom.g &&
		    pixel.b == rules.pupilFrom.b) {
			pixel.r = rules.pupilTo.r;
			pixel.g = rules.pupilTo.g;
			pixel.b = rules.pupilTo.b;
		}
	}
	if (rules.tint) {
		pixel.r = static_cast<std::uint8_t>(
			static_cast<int>(pixel.r) * rules.tintColor.r / 255);
		pixel.g = static_cast<std::uint8_t>(
			static_cast<int>(pixel.g) * rules.tintColor.g / 255);
		pixel.b = static_cast<std::uint8_t>(
			static_cast<int>(pixel.b) * rules.tintColor.b / 255);
	}
	return pixel;
}

void PlayerAppearancePainter::blendPixel(const int x, const int y,
					 const Rgba& color) const {
	if (color.a == 0 || x < 0 || y < 0 || x >= canvasWidth_ ||
	    y >= canvasHeight_) {
		return;
	}
	std::uint32_t& destination =
		canvas_[static_cast<std::size_t>(y) *
				static_cast<std::size_t>(canvasWidth_) +
			static_cast<std::size_t>(x)];
	if (color.a == 255) {
		destination = pack(color);
		return;
	}

	const Rgba under = unpack(destination);
	const int sourceAlpha = color.a;
	const int underAlpha = under.a;
	const int inverse = 255 - sourceAlpha;
	const int outAlpha = sourceAlpha + underAlpha * inverse / 255;
	if (outAlpha == 0) {
		destination = 0;
		return;
	}
	const auto blend = [&](const int source, const int base) {
		return (source * sourceAlpha +
			base * underAlpha * inverse / 255) /
		       outAlpha;
	};
	const Rgba out{
		static_cast<std::uint8_t>(blend(color.r, under.r)),
		static_cast<std::uint8_t>(blend(color.g, under.g)),
		static_cast<std::uint8_t>(blend(color.b, under.b)),
		static_cast<std::uint8_t>(outAlpha),
	};
	destination = pack(out);
}

std::uint32_t PlayerAppearancePainter::pack(const Rgba& color) const {
	return (static_cast<std::uint32_t>(color.a) << 24) |
	       (static_cast<std::uint32_t>(color.r) << 16) |
	       (static_cast<std::uint32_t>(color.g) << 8) |
	       static_cast<std::uint32_t>(color.b);
}

PlayerAppearancePainter::Rgba PlayerAppearancePainter::unpack(
	const std::uint32_t pixel) const {
	return Rgba{
		static_cast<std::uint8_t>((pixel >> 16) & 0xFF),
		static_cast<std::uint8_t>((pixel >> 8) & 0xFF),
		static_cast<std::uint8_t>(pixel & 0xFF),
		static_cast<std::uint8_t>((pixel >> 24) & 0xFF),
	};
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
	for (std::size_t i = 0; i < rules.skinWitness.size(); ++i) {
		rules.skinWitness[i] =
			sample(sheet, static_cast<int>(i), 0);
		rules.skinTarget[i] =
			sample(sheet, static_cast<int>(i), row);
	}
	rules.skinRecolor = true;
}

void PlayerAppearancePainter::readSleeveRules(
	SpriteSheet& shirtSheet, const world::SheetRegion& region,
	PixelRules& rules) const {
	const int originX = region.at.col * shirtSheet.frameWidth();
	const int originY = region.at.row * shirtSheet.frameHeight();
	for (std::size_t i = 0; i < kSleeveMarkers.size(); ++i) {
		rules.sleeveMarker[i] = Rgba{kSleeveMarkers[i][0],
					     kSleeveMarkers[i][1],
					     kSleeveMarkers[i][2], 255};
	}
	rules.sleeveTone[0] =
		sample(shirtSheet, originX + kSleeveSampleX,
		       originY + kSleeveDarkY);
	rules.sleeveTone[1] =
		sample(shirtSheet, originX + kSleeveSampleX,
		       originY + kSleeveMidY);
	rules.sleeveTone[2] =
		sample(shirtSheet, originX + kSleeveSampleX,
		       originY + kSleeveLightY);
	rules.sleeveReplace = true;
}

}  // namespace sdl
