#pragma once

#include "world/DialogCatalog.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura de un diálogo: dónde se dibuja sobre la entidad y dónde
// va su contenido, centrado. Solo rectángulos de mundo (sin SDL): la
// capa sdl/ pinta lo que esta clase calcula (§9).
class DialogLayout {
public:
	explicit DialogLayout(const DialogData& data);

	// Rect del diálogo: el tamaño de su frame dibujado, anclado al
	// offset sobre la caja dibujada de la entidad (Y negativa = arriba).
	PixelRect dialogRect(const PixelRect& entityBox) const;

	// Rect del contenido (ej. la leche), centrado dentro del diálogo.
	PixelRect contentRect(const PixelRect& dialogBox) const;

private:
	DialogData data_;
};

}  // namespace world
