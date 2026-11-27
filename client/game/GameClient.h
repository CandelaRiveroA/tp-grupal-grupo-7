#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "common/common_command.h"
#include "game/ContainerTransfer.h"
#include "game/FurnitureGrab.h"
#include "game/FurniturePlacement.h"
#include "game/InventoryDiscard.h"
#include "game/LocalWorldView.h"

namespace game {

class GameClient {
public:
    virtual ~GameClient() = default;

    // ENVIAR: pushea la intención del jugador.
    virtual void sendCommand(const protocol::ClientMessage& message) = 0;

    // CHAT: sendChat es la salida, receiveChat la entrada. Default vacío:
    // MockWorldClient los delega y los stubs de test no los implementan.
    virtual void sendChat(const std::string& /*text*/) {}

    virtual void receiveChat(std::uint8_t /*playerId*/, const std::string& /*characterName*/,
                             const std::string& /*text*/) {}

    virtual void requestTransfer(const ContainerTransfer& /*transfer*/) {}

    // Intención de colocar el mueble de un slot en una celda. Default
    // vacío: la fuente decide si se puede (hoy solo la demo la atiende).
    virtual void requestPlace(const FurniturePlacement& /*placement*/) {}

    virtual void requestGrab(const FurnitureGrab& /*grab*/) {}

    virtual void requestDiscard(const InventoryDiscard& /*discard*/) {}

    // RECIBIR: el estado actual del mundo que dibuja el cliente.
    virtual const LocalWorldView& worldState() const = 0;

    virtual bool connected() const { return true; }

    // Tick de la simulación: UNA llamada por iteración del loop, al
    // arrancar el tick (ver connection-point.md §4, paso 1). Avanza lo
    // que corre del lado de la fuente de estado: hoy, la simulación de
    // animales del mock; mañana, la implementación de red drena acá su
    // Receiver.
    virtual void update(int iteration) = 0;
};

}  // namespace game
