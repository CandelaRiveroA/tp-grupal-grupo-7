#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/MarketOffer.h"

namespace world {

// Vocabulario de los lados del mercado: la traducción entre el nombre
// que usa market.toml ("sells", "buys") y world::MarketSide. UNA sola
// tabla (convención §4/§9): ni switch ni mapas repetidos.
class MarketSideNames {
public:
	MarketSideNames();

	// nullopt si el nombre no pertenece al vocabulario.
	std::optional<MarketSide> find(const std::string& name) const;

	// Todo el vocabulario (para recorrer las claves de [sides]).
	const std::map<std::string, MarketSide>& entries() const;

private:
	std::map<std::string, MarketSide> sides_;
};

}
