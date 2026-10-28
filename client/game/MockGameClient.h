#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "world/ResourceCatalog.h"

namespace game {

class MockGameClient : public GameClient {
public:
	MockGameClient(int gridColumns, int gridRows, int startX,
		       int startY, int toolUseDurationTicks);

	void sendCommand(const protocol::Command& command) override;
	const LocalWorldView& worldState() const override;

private:
	// Mineral simulado: a los 3 golpes se agota. Simula al servidor.
	struct MineralSpot {
		world::ResourceKind kind;
		int cellX;
		int cellY;
		int hits = 0;
	};

	void walkTo(int targetX, int targetY, protocol::Direction direction);

	void rebuildView() const;

	void rebuildInventory();

	void rebuildResources() const;

	void rebuildDropped() const;

	void placeMinerals();

	int mineralAt(int cellX, int cellY) const;

	int droppedAt(int cellX, int cellY) const;

	// Recoge el item dropeado en la celda del jugador (si lo hay),
	// programa el evento de obtención y suma el mineral al inventario.
	void collectAt(int cellX, int cellY);

	// Suma +1 al slot con ese contentId; si no existe, lo crea en el
	// primer slotIndex libre. Simula al servidor acumulando cantidad.
	void addToInventory(const std::string& contentId);

	// contentId estable de un recurso (mismo vocabulario que el servidor
	// y que inventory.toml / resources.toml).
	static std::string contentIdFor(world::ResourceKind kind);

	// const + mutable porque worldState() es const por contrato de
	// GameClient, pero el mock avanza su simulación en cada consulta.
	void advanceToolUse() const;

	// Emite el evento de obtención (si fue encolado por un Take) por un
	// solo tick. El servidor decidirá este evento; esto lo simula.
	void advanceObtained() const;

	int gridColumns_;
	int gridRows_;

	int playerCellX_;
	int playerCellY_;
	protocol::Direction playerFacing_;
	bool playerMoving_;

	int toolUseDurationTicks_;
	mutable int toolUseRemaining_ = 0;

	// El golpe pendiente se confirma al completar la ventana (fin de la
	// animación), como hará el servidor.
	mutable std::vector<MineralSpot> minerals_;
	mutable int pendingMineral_ = -1;

	// Mineral caído al piso tras agotarse una roca. Recogible con Take.
	struct DroppedItem {
		world::ResourceKind kind;
		int cellX;
		int cellY;
	};
	mutable std::vector<DroppedItem> dropped_;

	// Evento de obtención encolado por un Take exitoso: se emite en el
	// próximo worldState() y dura un solo tick.
	mutable bool obtainedPending_ = false;
	mutable world::ResourceKind obtainedKind_{};

	mutable LocalWorldView view_;
};

}
