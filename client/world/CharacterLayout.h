#pragma once

#include "world/CharacterAppearance.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura del personaje. No conoce SDL ni assets: calcula dónde
// cae el cuerpo dentro de su box y el rectángulo de cada capa del pose,
// para que CharacterSprite (y quien lo reemplace) dibuje sin reimplementar
// la aritmética. Todas las capas se anclan abajo: su borde inferior
// coincide con bodyBottom.
class CharacterLayout {
public:
	CharacterLayout(const PixelRect& bodyBox, int drawnWidth,
			int drawnHeight);

	int bodyX() const;
	int bodyTop() const;
	int bodyBottom() const;
	int drawnWidth() const;
	int drawnHeight() const;

	// Rectángulo de una capa dentro del box. El factor de píxeles por
	// celda sale de la capa base del pose —el cuerpo—, no de la capa que
	// se está dibujando. Así una capa 1x1 mide lo que corresponde aunque
	// el cuerpo sea 1x2. Si una capa es inválida devuelve un rect vacío,
	// sin dividir por cero: el loader ya valida, esto solo evita romperse.
	PixelRect layerRect(const LayerFrame& layer,
			    const LayerFrame& baseLayer) const;

private:
	PixelRect bodyBox_;
	int drawnWidth_;
	int drawnHeight_;
};

}  // namespace world
