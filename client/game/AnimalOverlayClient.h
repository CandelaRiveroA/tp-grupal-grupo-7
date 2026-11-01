#pragma once

#include <memory>
#include <vector>

#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/Grid.h"

namespace game {

class MockAnimalSimulation;

// Decorator de GameClient: la fuente de estado real (el server por red,
// el mock en tests) manda, y esta capa SUPERPONE la simulación de
// animales (wander, sonidos, producción) sobre la vista que devuelve.
// Los animales hoy viven en el cliente; cuando el server los tenga, se
// borra esta capa y solo cambia quién llena la vista (contrato §6).
//
// La vista fusionada se arma en update() (la del último update, como
// pide el contrato de GameClient): la del inner queda intacta, esta
// capa agrega entidades, ítems de producción y sonidos encima.
class AnimalOverlayClient : public GameClient {
public:
	AnimalOverlayClient(std::unique_ptr<GameClient> inner,
			    const world::CharacterCatalog& characters,
			    world::GridSpan grid);

	// Destructor en el .cpp: MockAnimalSimulation está forward-declarado.
	~AnimalOverlayClient() override;

	void sendCommand(const protocol::ClientMessage& message) override;
	void update(int iteration) override;
	const LocalWorldView& worldState() const override;

private:
	void spawnAnimals(const world::CharacterCatalog& characters);

	// Toma lo que la simulación produjo en este tick y lo publica como
	// ítem del suelo (reglas del "server falso": tope y una celda un
	// ítem).
	void collectProducedItems();

	// contentId de un producto de la simulación (huevo, leche).
	std::string contentIdFor(world::ItemKind kind) const;

	// Celda del jugador en la vista del inner (centro del mundo si
	// todavía no hay entidades).
	void playerCell(int& cellX, int& cellY) const;

	void mergeView();

	std::unique_ptr<GameClient> inner_;
	std::unique_ptr<MockAnimalSimulation> simulation_;
	world::GridSpan grid_;

	// Productos de la simulación en el piso: estado propio de esta
	// capa; la fuente de estado real no los conoce.
	std::vector<ViewDroppedItem> groundItems_;

	// Sonidos de la simulación del último update (per-tick, drenados).
	std::vector<ViewSoundEvent> soundEvents_;

	// Vista fusionada: la del inner + lo que aporta esta capa. Se arma
	// en update() (la del último update, como pide el contrato).
	LocalWorldView merged_;
};

}  // namespace game
