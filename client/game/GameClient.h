#pragma once

#include "common/common_command.h"
#include "game/LocalWorldView.h"

namespace game {

class GameClient {
public:
    virtual ~GameClient() = default;

    virtual void sendCommand(const protocol::ClientMessage& message) = 0;

    // Avanza el estado un tick: drena lo recibido y actualiza la vista.
    // Se llama una vez por tick, antes de consultar worldState().
    virtual void update() = 0;

    // Consulta la vista del último update(). No avanza el estado.
    virtual const LocalWorldView& worldState() const = 0;
};

}  // namespace game
