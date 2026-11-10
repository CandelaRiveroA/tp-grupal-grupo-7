#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "sdl/context/SdlConstants.h"
#include "world/CharacterAppearance.h"

namespace game {
struct ViewEntity;
}

namespace world {
class CharacterCatalog;
struct CharacterPalettes;
}  // namespace world

namespace sdl {

class SdlContext;
class SheetCache;
class SpriteSheet;

// Compone la apariencia personalizable del jugador (piel, pantalón, camisa,
// ojos, accesorio y pelo) sobre el cuerpo/brazos del pose que resolvió
// world::AppearanceResolver. Trabaja a la resolución nativa del cuerpo
// (16x32) y escala el lienzo al box de pantalla, así los offsets del TOML
// son píxeles de arte y no dependen del zoom.
class PlayerAppearancePainter {
public:
	PlayerAppearancePainter(SdlContext& context, SheetCache& sheets,
				const world::CharacterCatalog& characters,
				const world::CharacterPalettes& palettes);

	void draw(const game::ViewEntity& entity, const world::Pose& pose,
		  world::ActionKind action, int frameIndex,
		  const SDL2pp::Rect& dest, Flip flip = Flip::None) const;

private:
	struct Rgba {
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
		std::uint8_t a = 0;
	};

	// Transformaciones por píxel al copiar una capa: recoloreo de piel,
	// reemplazo de las mangas testigo, pupilas y tinte multiplicativo.
	struct PixelRules {
		bool skinRecolor = false;
		std::array<Rgba, 3> skinWitness{};
		std::array<Rgba, 3> skinTarget{};
		bool sleeveReplace = false;
		std::array<Rgba, 3> sleeveMarker{};
		std::array<Rgba, 3> sleeveTone{};
		bool tint = false;
		Rgba tintColor{255, 255, 255, 255};
		bool pupilRecolor = false;
		Rgba pupilFrom{};
		Rgba pupilTo{};
	};

	void clearCanvas(int width, int height) const;
	void blit(SpriteSheet& sheet, const world::SheetRegion& region,
		  int offsetX, int offsetY, const PixelRules& rules) const;
	void recolorCanvas(const PixelRules& rules) const;

	Rgba sample(SpriteSheet& sheet, int x, int y) const;
	Rgba applyRules(Rgba pixel, const PixelRules& rules) const;
	void blendPixel(int x, int y, const Rgba& color) const;

	std::uint32_t pack(const Rgba& color) const;
	Rgba unpack(std::uint32_t pixel) const;

	const world::AppearanceOption* optionAt(
		const std::vector<world::AppearanceOption>& options,
		int index) const;
	const world::SheetRegion* regionAt(
		const world::AppearanceOption* option,
		world::AppearanceSide side) const;

	SpriteSheet& layerSheet(
		const world::AppearanceLayerData& layer) const;

	// Dónde se dibuja una capa: su offset por cara, interpretado contra su
	// ancla —cuerpo, coronilla o pies— para que siga a la parte del cuerpo
	// que le corresponde cuando se mueve entre poses.
	std::array<int, 2> placedAt(const world::AppearanceLayerData& layer,
				    std::size_t sideIndex, int crownY,
				    int feetY) const;

	void readSkinRules(const world::SkinAppearanceData& skin, int index,
			   PixelRules& rules) const;
	void readSleeveRules(SpriteSheet& shirtSheet,
			     const world::SheetRegion& region,
			     PixelRules& rules) const;

	SdlContext& context_;
	SheetCache& sheets_;
	const world::CharacterCatalog& characters_;
	const world::CharacterPalettes& palettes_;

	// Tabla ancla -> referencia vertical (0 / coronilla / pies) que sigue su
	// offset. Mapa sobre switch, inicializado en el constructor.
	std::map<world::AppearanceAnchor, std::function<int(int, int)>>
		anchorBase_;

	mutable std::vector<std::uint32_t> canvas_;
	mutable int canvasWidth_ = 0;
	mutable int canvasHeight_ = 0;
	mutable std::optional<SDL2pp::Texture> texture_;
	mutable int textureWidth_ = 0;
	mutable int textureHeight_ = 0;
};

}  // namespace sdl
