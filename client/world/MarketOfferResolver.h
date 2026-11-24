#pragma once

#include <optional>
#include <vector>

#include "world/MarketCatalog.h"
#include "world/MarketOffer.h"
#include "world/MarketPopupStyle.h"

namespace world {

// Decide QUÉ se muestra de una oferta: traduce lo que manda la fuente de
// estado (world::MarketOffer) a las secciones del popup, con el nombre y
// el título que SDL tiene cargados (MarketCatalog). Es lógica pura, sin
// SDL: no sabe de tamaños ni de páginas.
//
//  - Sin entradas usa la oferta por defecto del comerciante.
//  - Los productos que SDL no conoce se omiten.
//  - Un monto negativo no se muestra (queda solo el nombre).
//  - Si un producto viene repetido en el mismo lado, vale el primero.
//  - Las entradas del mismo producto con distinta calidad se juntan en
//    UN ítem con una línea por calidad (de menor a mayor); una calidad
//    que SDL no conoce se omite.
//  - Las secciones salen en el orden de world::MarketSide.
class MarketOfferResolver {
public:
	explicit MarketOfferResolver(const MarketCatalog& catalog);

	// Vacío si no hay nada para mostrar.
	std::vector<MarketSection> sectionsFrom(const MarketOffer& offer) const;

private:
	// La línea de calidad de una entrada con calidad; nullopt si SDL no
	// conoce esa calidad.
	std::optional<MarketTier> tierOf(const MarketEntry& entry) const;

	const MarketCatalog& catalog_;
};

}
