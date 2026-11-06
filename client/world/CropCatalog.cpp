#include "world/CropCatalog.h"

#include <cstddef>

namespace world {

int CropDefinition::finalStage() const {
	return static_cast<int>(growth.size()) + 1;
}

const SpriteVisual* CropDefinition::spriteForStage(
	const int stage, const CropQuality quality) const {
	if (stage < 0) {
		return nullptr;
	}
	if (stage == 0) {
		return &seed;
	}
	if (stage == finalStage()) {
		// La cosecha: su apariencia la da la calidad (si falta la
		// pedida, la base; si no hay ninguna, no hay sprite).
		const auto it = qualities.find(quality);
		if (it != qualities.end()) {
			return &it->second;
		}
		const auto base = qualities.find(CropQuality::Base);
		return base != qualities.end() ? &base->second : nullptr;
	}
	const std::size_t index = static_cast<std::size_t>(stage - 1);
	return index < growth.size() ? &growth[index] : nullptr;
}

void CropCatalog::setEntry(const std::string& cropId,
			   CropDefinition definition) {
	entries_[cropId] = std::move(definition);
}

const CropDefinition* CropCatalog::find(const std::string& cropId) const {
	const auto it = entries_.find(cropId);
	return it == entries_.end() ? nullptr : &it->second;
}

const std::map<std::string, CropDefinition>& CropCatalog::entries() const {
	return entries_;
}

void CropCatalog::registerContents(ContentCatalog& content,
				   ContentTraits& traits) const {
	for (const auto& [cropId, definition] : entries_) {
		content.visuals.emplace(definition.sackId, definition.sack);
		traits.set(definition.sackId,
			   ContentTrait{definition.droppable});
		// El producto cosechado se dibuja como la cosecha (hoy la
		// calidad base la manda la fuente; mañana el servidor
		// decidirá cuándo es increíble).
		const SpriteVisual* harvest = definition.spriteForStage(
			definition.finalStage(), CropQuality::Base);
		if (harvest != nullptr) {
			content.visuals.emplace(cropId, *harvest);
			traits.set(cropId,
				   ContentTrait{definition.droppable});
		}
	}
}

const std::string* CropCatalog::cropIdForSack(
	const std::string& sackId) const {
	for (const auto& [cropId, definition] : entries_) {
		if (definition.sackId == sackId) {
			return &cropId;
		}
	}
	return nullptr;
}

void CropCatalog::setPlantScale(const double scale) { plantScale_ = scale; }

double CropCatalog::plantScale() const { return plantScale_; }

void CropCatalog::setHarvestScale(const double scale) {
	harvestScale_ = scale;
}

double CropCatalog::harvestScale() const { return harvestScale_; }

}  // namespace world
