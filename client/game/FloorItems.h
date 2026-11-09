#pragma once

#include <vector>

#include "game/Interactor.h"
#include "world/ContentIds.h"
#include "world/ItemKind.h"

namespace game {

// Un ítem del piso del mundo: DATOS puros (qué es y dónde).
struct FloorItem {
	world::ItemKind kind;
	int cellX;
	int cellY;
};

// El piso del mundo: los ítems caídos y sus reglas (tope, una celda un
// ítem). Los sistemas (animales hoy; cultivos y enemigos mañana)
// PRODUCEN y esta clase valida y publica. Implementa Interactor: su
// objetivo es el ítem más cercano, y tocarlo lo recoge (queda en las
// manos del jugador, sale del piso).
class FloorItems : public Interactor {
public:
	// Intenta dejar un ítem en el piso. false si no se puede (tope
	// cumplido o celda ocupada): regla silenciosa, no un error.
	bool tryDrop(world::ItemKind kind, int cellX, int cellY);

	const std::vector<FloorItem>& items() const;

	// --- Interactor ------------------------------------------------
	// targetId = el índice del ítem al encontrarlo; onInteraction-
	// Started lo saca del piso. La cercanía es adyacente o igual.
	std::optional<InteractionTarget> findTargetNear(
		int cellX, int cellY) const override;
	void onInteractionStarted(const InteractionTarget& target) override;
	void onInteractionFinished(const InteractionTarget& target) override;

private:
	std::vector<FloorItem> items_;
	// Vocabulario contentId para la recompensa del objetivo.
	world::ContentIds contentIds_;
};

}  // namespace game
