#pragma once

#include <optional>
#include <string>

#include "game/CarrySelection.h"
#include "world/FurnitureFacing.h"
#include "world/FurnitureViews.h"

namespace game {

// Hacia dónde mira el mueble que el jugador está por colocar: lo elige
// con el click derecho. Presentación LOCAL de la selección (como
// CarrySelection): vuelve a frente al cambiar lo sostenido o el slot y
// al colocar. Se observa una vez por tick.
class FurnitureOrientation {
public:
	void observe(const CarrySelection& carry, int activeSlot);

	// Pasa a la siguiente orientación que el mueble sostenido tenga. Sin
	// mueble en la mano no hace nada.
	void rotate(const world::FurnitureViews& views);

	// Vuelve a frente (al colocar).
	void reset();

	world::FurnitureFacing facing() const;

private:
	std::optional<std::string> held_;
	int slot_ = -1;
	world::FurnitureFacing facing_ = world::FurnitureFacing::Front;
};

}
