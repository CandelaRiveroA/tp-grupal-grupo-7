#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace game {

// Qué pide el click sobre un edificio (solo datos). Qué hacer con eso lo
// decide quien llama.
struct StructureClick {
	// El click fue sobre un edificio con comportamiento (consumido: no
	// cae al mundo).
	bool hit = false;

	// El jugador está en el anillo del edificio SEGÚN LA VISTA: solo
	// entonces el click hace algo (como la cama, no desde cualquier
	// lado).
	bool inRange = false;

	// El edificio tocado; válido hasta el próximo reindex.
	const world::SceneStructure* structure = nullptr;
};

// Los edificios clickeables de la escena actual (los de rol con
// comportamiento en el overlay). Su caja de click es la MISMA que usa el
// renderer para dibujarlos (ElementLayout): si el editor mueve el
// edificio, la caja lo acompaña. No conoce SDL ni dibuja.
class StructureSelection {
public:
	StructureSelection(const world::SceneDescription& scene,
			   int cellPixelSize);

	// Re-indexa con la escena nueva (al cambiar de escena).
	void reindex(const world::SceneDescription& scene, int cellPixelSize);

	StructureClick clickAt(const world::PixelPoint& worldPoint,
			       const LocalWorldView& view) const;

	// ¿Hay un edificio apretable bajo el punto (variante del cursor)?
	// Apretable = el click hace algo: el jugador está cerca.
	bool hits(const world::PixelPoint& worldPoint,
		  const LocalWorldView& view) const;

private:
	struct Entry {
		world::PixelRect bounds;
		world::SceneStructure structure;
	};

	// Último en el orden de dibujo que contiene el punto (el de más
	// arriba), o nullptr.
	const Entry* entryAt(const world::PixelPoint& worldPoint) const;

	bool near(const Entry& entry, const LocalWorldView& view) const;

	std::vector<Entry> entries_;
};

}
