#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/CropQuality.h"

namespace world {

// Vocabulario de las calidades: la traducción entre el nombre que usa un
// TOML ("base", "good", ...) y world::CropQuality. UNA sola tabla
// (convención §4/§9).
class CropQualityNames {
public:
	CropQualityNames();

	// nullopt si el nombre no pertenece al vocabulario.
	std::optional<CropQuality> find(const std::string& name) const;

	// Todo el vocabulario (para recorrer las claves de [qualities]).
	const std::map<std::string, CropQuality>& entries() const;

private:
	std::map<std::string, CropQuality> qualities_;
};

}
