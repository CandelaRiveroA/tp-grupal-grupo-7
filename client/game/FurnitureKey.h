#pragma once

#include <string>

#include "game/LocalWorldView.h"

namespace game {

// Identidad de UN mueble colocado (su contentId y su celda base): la
// clave con la que se lleva el estado animado de cada instancia (dos
// cofres iguales se abren por separado). Una sola fuente de la clave,
// para que quien anima y quien dibuja hablen de lo mismo.
class FurnitureKey {
public:
	std::string of(const ViewFurniture& furniture) const;
};

}
