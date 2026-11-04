#pragma once

#include <map>
#include <vector>

#include "world/Indication.h"
#include "world/ScenePixelMapper.h"
#include "world/SpriteVisual.h"

namespace world {

// Animación de aparición de un diálogo: secuencia de celdas sobre una
// hoja común (sheet/cell/drawn en `sprite`) + su ritmo. One-shot por
// defecto: avanza hasta el último frame y se queda ahí ("informa").
struct DialogAnimation {
	// Hoja, celda y tamaño de los frames (sus col/row no se usan: la
	// celda de cada frame está en `frames`).
	SpriteVisual sprite;
	std::vector<SheetCell> frames;
	int frameTicks = 1;
	bool loops = false;

	// Índice del frame que corresponde a un tick de animación.
	int frameIndexAt(int animationTick) const;
};

// Un diálogo del juego como DATOS: cómo aparece (animación), qué
// muestra adentro (contenido centrado) y desde dónde se ancla a la
// entidad. Cambiar lo que muestra el diálogo = datos, no código.
struct DialogData {
	DialogAnimation animation;

	// Lo que aparece dentro del diálogo (ej. la leche), centrado.
	SpriteVisual content;

	// Corrección del contenido respecto de su centro (px; Y negativa =
	// más arriba). Óptica: los sprites con padding transparente se ven
	// descentrados sin esto.
	PixelPoint contentOffset;

	// Offset en px desde la esquina superior izquierda de la caja
	// dibujada de la entidad (negativo en Y = sobre ella).
	PixelPoint anchorOffset;
};

struct DialogCatalog {
	std::map<Indication, DialogData> dialogs;

	// Una sola búsqueda: nullptr si la indicación no tiene diálogo.
	const DialogData* find(Indication indication) const;
};

}  // namespace world
