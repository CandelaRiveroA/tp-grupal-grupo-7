#pragma once

// Capas que componen el personaje, en orden de dibujo: de abajo hacia arriba.
// El pelo va arriba de todo. HairColor y PantsColor son tintes: no se dibujan.
enum class CharacterLayer {
	Skin,
	Pants,
	PantsColor,
	Shirt,
	Eyes,
	Accessory,
	HairColor,
	Hair,
	Count,
};
