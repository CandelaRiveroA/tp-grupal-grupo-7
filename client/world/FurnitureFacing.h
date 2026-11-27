#pragma once

namespace world {

// Hacia dónde mira un mueble colocado. Es vocabulario del juego (mañana
// viaja al server con la colocación): qué sprite lo dibuja en cada caso
// es dato (las vistas `side` / `back` de inventory.toml), ver
// world::FurnitureViews.
enum class FurnitureFacing {
	Front,
	Right,
	Back,
	Left,
};

}
