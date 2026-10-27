#pragma once

#include <cstddef>
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

	void placeMinerals();

	int mineralAt(int cellX, int cellY) const;

	// const + mutable porque worldState() es const por contrato de
	// GameClient, pero el mock avanza su simulación en cada consulta.
	void advanceToolUse() const;

	// El evento de obtención se emite unos ticks DESPUÉS del crack, para
	// que el discover suene luego del quiebre y no encimado. El servidor
	// decidirá ese timing; esto lo simula.
	void advanceObtained() const;

	static constexpr int kObtainedDelayTicks = 18;

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

	mutable int obtainedCountdown_ = 0;
	mutable world::ResourceKind obtainedKind_{};

	mutable LocalWorldView view_;
};

}
