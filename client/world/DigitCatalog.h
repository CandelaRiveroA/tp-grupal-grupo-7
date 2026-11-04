#pragma once

#include <array>
#include <string>
#include <vector>

#include "world/SpriteVisual.h"

namespace world {

// Hoja de dígitos (0-9) descrita como DATOS: de qué imagen salen y qué
// celda es cada dígito. Permite componer cualquier número (la cantidad
// del inventario, de 1 a N; el límite lo pone el servidor). No depende de
// SDL. Lo carga world::DigitConfigLoader desde digits.toml; el HUD lo
// consume para dibujar el número de cantidad arriba-derecha del slot.
struct DigitCatalog {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;

	// Indexado por valor del dígito: digits[0]..digits[9].
	std::array<SheetCell, 10> digits{};

	// Celda del dígito d (0-9). Precondición: 0 <= d <= 9.
	const SheetCell& cellFor(int digit) const;
};

// Descompone n en sus dígitos decimales, de más significativo a menos
// (ej. 21 -> {2,1}, 5 -> {5}, 0 -> {0}). Negativos se tratan como 0.
// Función pura: base para dibujar el número sin lógica en el HUD.
std::vector<int> digitsOf(int n);

}  // namespace world
