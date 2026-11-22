#pragma once

#include "common/common_command.h"
#include "world/ViewDirection.h"

namespace game {

class MoveCommand {
public:
    // Caminar hacia una dirección (fija también el facing del jugador).
    protocol::ClientMessage propose(world::Direction direction) const;

    // Picar: usar la herramienta del casillero `slot` sobre la celda enfrentada.
    protocol::ClientMessage use(world::Direction direction, int slot) const;

    // Recoger un item del piso: apunta a la celda PROPIA del jugador. La
    // fuente decide si hay algo para recoger. El casillero activo viaja
    // (como en Pick): el server lo usa para saber qué tiene equipado.
    protocol::ClientMessage take(world::Direction direction, int slot) const;
};

}  // namespace game
