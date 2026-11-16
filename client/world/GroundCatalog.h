#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "world/Grid.h"
#include "world/SpriteVisual.h"

namespace world {

// Consecuencia aplicada al terreno de una celda (vocabulario del juego,
// lo comunica el servidor). Agregar una consecuencia nueva (regar) = un
// valor acá + su entrada en ground.toml: el dibujo no cambia.
enum class GroundConsequence {
	// Tierra tratada (suelo tratado): el piso queda listo para sembrar.
	Dug,
	// Regada: la tierra tratada queda regada (el set oscuro de hoeDirt).
	Watered,
	// Charco de sangre: la salpicadura tintada de Animations.png a los
	// pies (hoy la deja la demo de la tecla C; mañana la fuente).
	Blood,
};

// Cómo se ve una consecuencia de terreno: el sprite simple (fallback y
// para consecuencias que no se arman por zonas) y, opcionalmente, las
// PIEZAS con que se arma una zona conectada (auto-tiling por máscara
// de vecinos: CellPatch). Cada pieza puede tener varias versiones
// dibujadas (variantes).
struct GroundVisual {
	SpriteVisual sprite;
	// Máscara de vecinos (kNeighbor* de WorldConstants.h) -> variantes
	// de esa pieza. Vacía = todo se dibuja con `sprite`.
	std::map<std::uint8_t, std::vector<SpriteVisual>> pieces;
};

// Catálogo consecuencia -> visual. Lo consulta el painter, nunca decide
// él qué se dibuja.
class GroundCatalog {
public:
	void setConsequence(GroundConsequence consequence, GroundVisual visual);

	// nullptr si la consecuencia no tiene visual cargada.
	const GroundVisual* find(GroundConsequence consequence) const;

	// Qué sprite dibuja una celda de la consecuencia según su máscara
	// de vecinos: la pieza de esa máscara, o el sprite simple si no la
	// tiene. La variante se elige por la POSICIÓN de la celda
	// (estable: la misma celda siempre muestra la misma versión).
	const SpriteVisual* pieceFor(GroundConsequence consequence,
				     std::uint8_t mask,
				     const GridCell& cell) const;

private:
	std::map<GroundConsequence, GroundVisual> visuals_;
};

}
