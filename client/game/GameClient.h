#pragma once

#include <memory>

#include "common/common_command.h"
#include "game/LocalWorldView.h"

namespace game {

class GameClient {
public:
    virtual ~GameClient() = default;

    // ENVIAR: pushea la intención del jugador.
    virtual void sendCommand(const protocol::ClientMessage& message) = 0;

    // RECIBIR: el estado actual del mundo que dibuja el cliente.
    virtual const LocalWorldView& worldState() const = 0;

    // Tick de la simulación: UNA llamada por iteración del loop, al
    // arrancar el tick (ver connection-point.md §4, paso 1). Avanza lo
    // que corre del lado de la fuente de estado: hoy, la simulación de
    // animales del mock; mañana, la implementación de red drena acá su
    // Receiver.
    virtual void update(int iteration) = 0;
};

}  // namespace game
