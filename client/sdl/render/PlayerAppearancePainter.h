#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <array>
#include <functional>
#include <map>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "sdl/context/SdlConstants.h"
#include "sdl/render/PixelComposer.h"
#include "world/CharacterAppearance.h"
#include "world/ViewDirection.h"

namespace game {
struct ViewEntity;
}

namespace world {
class CharacterCatalog;
struct CharacterPalettes;
enum class CreatureKind;
}  // namespace world

namespace sdl {

class SheetCache;
class SdlContext;
class SpriteSheet;

// Compone la apariencia personalizable del jugador (piel, pantalón, camisa,
// ojos, accesorio y pelo) sobre el cuerpo/brazos del pose que resolvió
// world::AppearanceResolver. Decide qué capa va en qué orden y con qué reglas;
// el combinado de píxeles lo hace PixelComposer. Trabaja a la resolución nativa
// del cuerpo (16x32) y escala el lienzo al box de pantalla, así los offsets del
// TOML son píxeles de arte y no dependen del zoom.
class PlayerAppearancePainter {
public:
	PlayerAppearancePainter(SdlContext& context, SheetCache& sheets,
				const world::CharacterCatalog& characters,
				const world::CharacterPalettes& palettes);

	void draw(const game::ViewEntity& entity, const world::Pose& pose,
		  world::ActionKind action, int frameIndex,
		  const SDL2pp::Rect& dest, Flip flip = Flip::None) const;

	// Compone al jugador en pose Front y dibuja solo el recorte de la
	// cabeza.
	void drawHead(const CharacterAppearance& appearance,
		      const SDL2pp::Rect& dest) const;

private:
	using Rgba = PixelComposer::Rgba;
	using PixelRules = PixelComposer::PixelRules;

	bool compose(world::CreatureKind kind, world::Direction facing,
		     const CharacterAppearance& characterAppearance,
		     const world::Pose& pose, world::ActionKind action,
		     int frameIndex) const;

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

	SheetCache& sheets_;
	const world::CharacterCatalog& characters_;
	const world::CharacterPalettes& palettes_;

	// Tabla ancla -> referencia vertical (0 / coronilla / pies) que sigue su
	// offset. Mapa sobre switch, inicializado en el constructor.
	std::map<world::AppearanceAnchor, std::function<int(int, int)>>
		anchorBase_;

	// Lienzo y textura de la composición. Mutable porque el painter dibuja
	// en un método const (el estado es sólo un buffer reutilizable).
	mutable PixelComposer composer_;
};

}  // namespace sdl
