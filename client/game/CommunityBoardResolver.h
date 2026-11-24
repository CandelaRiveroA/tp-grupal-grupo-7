#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "world/CommunityBoard.h"
#include "world/CommunityCatalog.h"

namespace game {

// Decide QUÉ se muestra del Centro Comunitario: cruza lo que SDL sabe
// mostrar (world::CommunityCatalog) con el progreso que manda la fuente
// de estado (LocalWorldView::communitySets) y arma las secciones del
// popup. Lógica pura, sin SDL.
//
//  - Un set o una meta que la fuente no menciona queda en 0 / 0 y sin
//    cumplir.
//  - Un set o una meta que el catálogo no conoce se ignora.
//  - Los estados (meta cumplida, set completo) los dice la fuente: acá
//    no se deducen de las cantidades.
//  - Una meta sin nombre muestra solo la cantidad.
class CommunityBoardResolver {
public:
	explicit CommunityBoardResolver(const world::CommunityCatalog& catalog);

	std::vector<world::CommunitySection> sectionsFrom(
		const LocalWorldView& view) const;

private:
	const world::CommunityCatalog& catalog_;
};

}
