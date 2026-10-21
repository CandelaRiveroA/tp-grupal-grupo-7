#pragma once

#include <string>

namespace core {

// Lo que el lobby (Qt) decide antes de delegar en el "main SDL".
// La validación por protocolo (partida inexistente / usuario tomado)
// se suma acá cuando el server la soporte.
struct SessionConfig {
	std::string partida;
	std::string personaje;
	int volumen = 50;
};

}
