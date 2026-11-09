#include "game/CropSystem.h"

#include <cstdlib>

#include "game/GameConstants.h"

namespace game {

CropSystem::CropSystem(const world::CropCatalog* crops) : crops_(crops) {}

void CropSystem::plant(const int cellX, const int cellY,
		       const std::string& cropId) {
	for (const ViewCrop& crop : planted_) {
		if (crop.cellX == cellX && crop.cellY == cellY) {
			return;
		}
	}
	// Se planta la semilla (stage 0) pidiendo agua: cada riego que
	// atiende el pedido la hace crecer una etapa (demo).
	ViewCrop crop{cellX, cellY, cropId, 0};
	crop.indication = world::Indication::ReadyToWater;
	planted_.push_back(crop);
}

void CropSystem::waterAt(const int cellX, const int cellY) {
	for (ViewCrop& crop : planted_) {
		if (crop.cellX != cellX || crop.cellY != cellY) {
			continue;
		}
		// Solo crece si pedía agua y le queda camino: en la etapa
		// final (la cosecha) queda con la calidad base por ahora
		// y lista para levantar.
		const world::CropDefinition* definition =
			crops_ != nullptr ? crops_->find(crop.cropId) : nullptr;
		if (definition == nullptr || !crop.indication.has_value() ||
		    crop.stage >= definition->finalStage()) {
			return;
		}
		crop.stage += 1;
		if (crop.stage >= definition->finalStage()) {
			crop.quality = world::CropQuality::Base;
			crop.indication.reset();
			crop.interactive = true;
		}
		return;
	}
}

const std::vector<ViewCrop>& CropSystem::planted() const { return planted_; }

std::optional<InteractionTarget> CropSystem::findTargetNear(
	const int cellX, const int cellY) const {
	for (std::size_t i = 0; i < planted_.size(); ++i) {
		const ViewCrop& crop = planted_[i];
		// Cerca = las celdas se tocan (adyacentes o la misma).
		if (!crop.interactive || std::abs(crop.cellX - cellX) > 1 ||
		    std::abs(crop.cellY - cellY) > 1) {
			continue;
		}
		InteractionTarget target;
		target.action = world::ActionKind::Pickup;
		target.cycles = kPickupCycles;
		// Lo que levanta es lo que entra al inventario: el
		// producto cosechado (el cropId), dibujado como la cosecha.
		// Se sostiene en las manos durante la ventana: el MISMO
		// mecanismo genérico que el huevo (overlay [held.*]).
		target.reward = crop.cropId;
		target.heldContent = crop.cropId;
		target.targetId = static_cast<int>(i);
		target.cellX = crop.cellX;
		target.cellY = crop.cellY;
		return target;
	}
	return std::nullopt;
}

void CropSystem::onInteractionStarted(const InteractionTarget& target) {
	// La cosecha queda en las manos: sale del mundo al empezar (el
	// mismo reflejo que el ítem del piso), y se la ve subir con el
	// overlay genérico mientras dura la ventana.
	if (target.targetId < 0 ||
	    target.targetId >= static_cast<int>(planted_.size())) {
		return;
	}
	planted_.erase(planted_.begin() + target.targetId);
}

void CropSystem::onInteractionFinished(const InteractionTarget&) {
	// Nada que hacer: la cosecha ya salió del mundo al empezar (el
	// producto lo entrega la sesión al inventario).
}

}
