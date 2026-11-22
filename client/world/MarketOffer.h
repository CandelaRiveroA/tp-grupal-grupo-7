#pragma once

#include <optional>
#include <string>
#include <vector>

namespace world {

// De qué lado del mostrador está un producto (vocabulario compartido con
// el servidor). Los títulos que lo acompañan son presentación: viven en
// market.toml ([sides]).
enum class MarketSide {
	Sells,
	Buys,
};

// Un producto de una oferta: qué es (contentId, el mismo del inventario),
// de qué lado va y, si la fuente lo manda, su monto. Sin monto solo se
// muestra el nombre.
struct MarketEntry {
	std::string contentId;
	MarketSide side = MarketSide::Sells;
	std::optional<int> price;
};

// Lo que un comerciante muestra. `merchantId` es el id de npcs.toml.
// Sin entradas = "mostrá lo tuyo": SDL usa la oferta por defecto del
// comerciante (market.toml). Con entradas, SDL muestra esas (las que
// conoce) con el monto que trae cada una.
struct MarketOffer {
	std::string merchantId;
	std::vector<MarketEntry> entries;
};

}
