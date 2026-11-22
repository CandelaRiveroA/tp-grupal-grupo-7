#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <cstdint>
#include <optional>
#include <vector>

#include "sdl/context/SdlConstants.h"

namespace world {
struct SheetRegion;
}

namespace sdl {

class SdlContext;
class SpriteSheet;

// Compone capas de sprites en un lienzo de píxeles nativo: copia regiones de
// una hoja, remapea colores testigo, tiñe y mezcla por alfa, y sube el
// resultado a una textura streaming para dibujarlo escalado. No decide QUÉ se
// compone: sólo cómo se combinan píxeles. Quién arma las reglas y en qué orden
// dibuja cada capa es responsabilidad de quien lo usa.
class PixelComposer {
public:
	struct Rgba {
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
		std::uint8_t a = 0;
	};

	// Remapeo de un color testigo a otro. Si `to` no es opaco, el testigo
	// existe pero esa capa no tiene reemplazo y se deja igual.
	struct Recolor {
		Rgba from{};
		Rgba to{};
	};

	// Transformaciones por píxel al copiar una capa: remapeos en orden y
	// tinte multiplicativo.
	struct PixelRules {
		std::vector<Recolor> recolors;
		bool tint = false;
		Rgba tintColor{255, 255, 255, 255};

		// Suma los remapeos de `other` después de los propios.
		void merge(const PixelRules& other);
	};

	explicit PixelComposer(SdlContext& context);

	// Deja el lienzo vacío del tamaño nativo pedido y prepara su textura.
	void reset(int width, int height);

	// Copia una región de la hoja en (offsetX, offsetY) del lienzo,
	// aplicando `rules` y mezclando por alfa.
	void drawRegion(SpriteSheet& sheet, const world::SheetRegion& region,
			int offsetX, int offsetY, const PixelRules& rules);

	// Aplica las reglas a todos los píxeles opacos del lienzo (por ejemplo,
	// recolorear pupilas que ya venían pintadas en el cuerpo).
	void recolorCanvas(const PixelRules& rules);

	// Color de un píxel de la hoja, para muestrear paletas y tonos.
	Rgba sample(SpriteSheet& sheet, int x, int y) const;

	// Sube el lienzo a la textura y lo dibuja escalado en `dest`.
	void present(const SDL2pp::Rect& dest, Flip flip);

private:
	Rgba applyRules(Rgba pixel, const PixelRules& rules) const;
	void blendPixel(int x, int y, const Rgba& color);

	std::uint32_t pack(const Rgba& color) const;
	Rgba unpack(std::uint32_t pixel) const;

	SdlContext& context_;

	std::vector<std::uint32_t> canvas_;
	int width_ = 0;
	int height_ = 0;
	std::optional<SDL2pp::Texture> texture_;
	int textureWidth_ = 0;
	int textureHeight_ = 0;
};

}  // namespace sdl
