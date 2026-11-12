#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "sdl/context/SdlConstants.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class SpriteSheet {
public:
	SpriteSheet(SdlContext& context, const std::string& path,
		    int frameWidth, int frameHeight);

	SpriteSheet(const SpriteSheet&) = delete;
	SpriteSheet& operator=(const SpriteSheet&) = delete;

	int frameWidth() const;
	int frameHeight() const;

	SDL2pp::Rect frameRect(int col, int row) const;

	void draw(int col, int row, const SDL2pp::Rect& dest,
		  Flip flip = Flip::None);

	// Las esquinas quedan de border×border (1:1) y los bordes y el
	// centro se estiran
	void drawNineSlice(int col, int row, int border,
			   const SDL2pp::Rect& dest);

	// Dibuja el CONTENIDO de la celda (sin su aire transparente) a la
	// MISMA escala que tendría la celda entera (multiplicada por
	// `scale`) y centrado en dest: el sprite conserva su proporción y
	// el aire se reparte parejo alrededor. Sirve para sprites con
	// margen variable dentro de su celda (las plantas de crops.toml,
	// cada una dibujada donde le cabe en la hoja).
	void drawContentCentered(int col, int row, const SDL2pp::Rect& dest,
				 double scale = 1.0);

	// Dibuja SOLO el contenido opaco de la celda estirado a dest, sin
	// conservar proporción. Para rellenos que crecen/achican en un eje
	// (la barra de energía): el degradé horizontal se estira al ancho
	// de la cavidad y la altura sigue la fracción.
	void drawContentStretched(int col, int row, const SDL2pp::Rect& dest);

	// Tiñe la hoja al dibujar (color modulation): cada píxel se multiplica
	// por (r,g,b). Útil para recolorear sprites claros (ej. dígitos
	// blancos -> negro con (0,0,0)). clearColorMod() vuelve a (255,255,255).
	void setColorMod(std::uint8_t r, std::uint8_t g, std::uint8_t b);
	void clearColorMod();

	void drawBlock(int col, int row, int cols, int rows,
		       const SDL2pp::Rect& dest, Flip flip = Flip::None,
		       double fracW = 1.0, double fracH = 1.0,
		       double angle = 0.0,
		       SDL2pp::Optional<SDL2pp::Point> center =
			       SDL2pp::NullOpt);

private:
	// Rectángulo del contenido con alfa dentro de la celda (en píxeles
	// de la hoja). La hoja se escanea una sola vez por celda pedida y
	// el resultado se cachea; una celda vacía o sin alfa consultable
	// devuelve la celda entera.
	SDL2pp::Rect contentRect(int col, int row);
	SDL2pp::Rect scanContentRect(int col, int row);

	SdlContext& context_;
	std::string path_;
	SDL2pp::Texture texture_;
	int frameWidth_;
	int frameHeight_;
	std::optional<SDL2pp::Surface> surface_;
	std::map<std::pair<int, int>, SDL2pp::Rect> contentRects_;
};

}
