#pragma once

namespace game {

class FootstepCue {
public:
	// stepFrame: cuadro de la acción con piernas que se está animando.
	// Suena el paso al empezar a moverse y cada vez que cambia de
	// cuadro (una pisada por cambio). Genérico: no conoce acciones.
	bool update(bool moving, int stepFrame);

private:
	bool moving_ = false;
	int frame_ = 0;
};

}
