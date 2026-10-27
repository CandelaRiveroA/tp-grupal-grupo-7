#pragma once

#include <memory>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"

namespace game {

class MockAnimalSimulation;

class MockGameClient : public GameClient {
public:
	// Mundo con SOLO el jugador (compatibilidad con pruebas existentes).
	MockGameClient(int gridColumns, int gridRows, int startX,
		       int startY);

	// Mundo con el jugador + simulación de animales (wander). El
	// catálogo aporta el ritmo de paso de cada criatura (datos TOML);
	// sin entrada Chicken, corre sin animales (sin romper).
	MockGameClient(int gridColumns, int gridRows, int startX,
		       int startY, const world::CharacterCatalog& characters);

	// El destructor se define en el .cpp: el unique_ptr de
	// MockAnimalSimulation necesita el tipo completo (está forward-declarado).
	~MockGameClient() override;

	void sendCommand(const protocol::Command& command) override;
	const LocalWorldView& worldState() const override;
	void update(int iteration) override;

private:
	void walkTo(int targetX, int targetY, protocol::Direction direction);

	void rebuildView();

	void rebuildInventory();

	void spawnAnimals(const world::CharacterCatalog& characters);

	// Toma los huevos que la simulación puso en este tick y los pasa
	// al estado de ítems del suelo (validando reglas del "server
	// falso": tope y celda ya ocupada por otro huevo).
	void collectLaidEggs();

	int gridColumns_;
	int gridRows_;

	int playerCellX_;
	int playerCellY_;
	protocol::Direction playerFacing_;
	bool playerMoving_;

	// La simulación de animales es del lado del "server falso": nula en
	// el mundo de solo jugador. Cada responsabilidad, en su clase.
	std::unique_ptr<MockAnimalSimulation> simulation_;

	// Ítems del suelo (ej. huevos que ponen los animales): estado del
	// "server falso", con ids estables propias. Sin simulación queda
	// vacío.
	std::vector<ViewGroundItem> groundItems_;
	int nextGroundItemId_;

	LocalWorldView view_;
};

}