#pragma once

#include <memory>
#include <set>
#include <vector>

#include "game/FloorItems.h"
#include "game/Interactor.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/ContentIds.h"
#include "world/Grid.h"

namespace game {

class MockAnimalSimulation;

// El dominio de animales del "server falso": su simulación (wander,
// producción, sonidos) y sus reglas — la vaca que da leche queda LISTA
// para ordeñar y se congela mientras tanto. Implementa Interactor: su
// objetivo es la vaca lista cerca del jugador y al terminar entrega
// leche. Cuando el server tenga animales, se borra este módulo y solo
// cambia quién llena la vista (contrato §6).
class AnimalSystem : public Interactor {
public:
	AnimalSystem(world::GridSpan grid, FloorItems& floor);
	~AnimalSystem() override;

	// Spawnea los animales del mundo (celdas en el .cpp). El jugador
	// está en su celda para no spawnear encima. `only`: si no está
	// vacío, spawnea SOLO esas especies (las demás las trae otra fuente,
	// p. ej. el server manda gallinas y vacas y acá van solo enemigos);
	// vacío = todas.
	void spawnAnimals(const world::CharacterCatalog& characters,
			  int playerCellX, int playerCellY,
			  const std::vector<world::CreatureKind>& only = {});

	void update(int iteration, int playerCellX, int playerCellY);

	// ANDAMIAJE TEMPORAL (prueba): enciende el ataque de los enemigos.
	// Mientras dura su ventana, appendEntities los reporta con
	// ActionKind::Attack; después vuelven a caminar/idle. El server real
	// pondrá la acción directo en la vista y esto se borra.
	void triggerEnemyAttack();

	// Las entidades de los animales para la vista (incluye la vaca
	// congelada mientras se la ordeña).
	void appendEntities(std::vector<ViewEntity>& out) const;

	// Sonidos del último update (per-tick, drenados).
	std::vector<ViewSoundEvent> takeSoundEvents();

	// --- Interactor (la vaca lista cerca) ---------------------------
	std::optional<InteractionTarget> findTargetNear(
		int cellX, int cellY) const override;
	void onInteractionStarted(const InteractionTarget& target) override;
	void onInteractionFinished(const InteractionTarget& target) override;

private:
	std::unique_ptr<MockAnimalSimulation> simulation_;
	// El piso del mundo: los huevos caen ahí.
	FloorItems* floor_;
	// Vocabulario contentId para la recompensa del objetivo.
	world::ContentIds contentIds_;

	// Animales listos para ordeñar (la vaca deja de tirar leche y
	// queda con su indicación). Reglas del "server falso".
	std::set<int> readyAnimals_;
	// La vaca congelada durante el ordeñe (su celda al empezar).
	int frozenCowId_ = -1;
	int frozenCowCellX_ = 0;
	int frozenCowCellY_ = 0;

	// ANDAMIAJE TEMPORAL (prueba del ataque de enemigos):
	// kinds que SABEN atacar (los que el catálogo describe con la acción
	// Attack), llenado al spawnear; y los ticks que quedan del ataque en
	// curso (0 = no atacando). Se va con la tecla de prueba.
	std::set<world::CreatureKind> attackers_;
	int attackTicksLeft_ = 0;

	std::vector<ViewSoundEvent> soundEvents_;
};

}  // namespace game
