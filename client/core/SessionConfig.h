#pragma once

#include <optional>
#include <string>

#include "common/common_socket.h"

namespace core {

// Lo que el lobby (Qt) decide antes de delegar en el "main SDL".
// La validación por protocolo (partida inexistente / usuario tomado)
// se suma acá cuando el server la soporte.
struct SessionConfig {
    std::string partida;
    std::string personaje;
    int volumen = 50;
    // Conexión ya unida a la partida en el servidor.
    std::optional<Socket> connection;
};

}  // namespace core
