#pragma once

#include <string>
#include <vector>

#include "game/Interactor.h"
#include "game/LocalWorldView.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace game {

// La interacción en curso de la capa del mundo simulado: la ventana
// (qué acción, cuánto, mirando a dónde, con qué en las manos) más su
// reflejo en la vista — la acción del jugador, el evento de obtenido y
// el inventario. UN único mecanismo para ordeñar, recoger, cosechar...
// (convención §9). Es estado de juego: la fuente la abre con su
// validación y el cliente solo la anima.
//
// La duración se mide con los MISMOS datos de la acción que usa la
// animación (contrato §10), así ventana y animación no se desfasan.
class InteractionSession {
public:
	// Abre la ventana sobre el objetivo, mirándolo desde la celda del
	// jugador. Duración = cycles × los datos de la acción (mínimo 1).
	void start(const InteractionTarget& target, int playerCellX,
		   int playerCellY, const world::CreatureData& creature);

	bool active() const;

	// Avanza un tick. Devuelve true solo en el tick en que la ventana
	// se cierra: el momento de entregar la recompensa.
	bool advance();

	const InteractionTarget& target() const;

	// Presentación: la acción/facing/en-manos del jugador mientras
	// dura; al terminar el facing se mantiene hasta que el jugador
	// haga otra cosa.
	void mergePlayer(ViewEntity& player);

	// Entrega: el obtenido (un tick) y el inventario con la regla
	// genérica de apilado por contentId. Se aplica sobre una vista
	// FRESCA de cada merge (la del inner + la capa, que se reconstruye
	// cada tick): por eso los conteos entregados se re-aplican.
	void mergeRewards(LocalWorldView& view);

private:
	// Una entrega acumulada: qué se obtuvo, en qué slot quedó LA
	// PRIMERA VEZ y cuántas unidades. Se guardan en ORDEN de obtención
	// y cada una conserva su slot: el inventario no se reordena nunca.
	struct InventoryGain {
		std::string contentId;
		int slotIndex = -1;
		int count = 0;
	};

	// Registra una unidad entregada (el que ya tenía gain conserva su
	// slot; el nuevo se agrega al final, en orden de obtención).
	void recordGain(const std::string& contentId);

	// Dirección dominante desde una celda hacia otra (mirar al
	// objetivo).
	world::Direction facingToward(int fromX, int fromY, int toX,
				      int toY) const;

	InteractionTarget target_;
	world::Direction facing_ = world::Direction::Front;
	int ticksRemaining_ = 0;
	bool facingOverride_ = false;
	bool rewardPending_ = false;
	std::vector<InventoryGain> inventoryGains_;
};

}  // namespace game
