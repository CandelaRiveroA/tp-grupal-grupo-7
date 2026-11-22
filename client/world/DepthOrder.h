#pragma once

#include <cstdint>
#include <vector>

namespace world {

enum class DepthKind {
	Element,
	Entity,
};

// Qué dibujar y en qué posición del orden: el índice es el del
// elemento de la escena o el de la entidad de la vista, según `kind`.
struct DepthItem {
	int row;
	DepthKind kind;
	std::uint32_t index;
};

// Orden de dibujo en profundidad (vista desde arriba): lo que está más
// abajo en el mapa se dibuja después y tapa a lo que está detrás. Un
// elemento (árbol, edificio) se ordena por su ÚLTIMA fila (su base) y
// una entidad por su fila. A igual fila el elemento va antes: quien
// está parado en la fila de la base se ve delante, y en filas
// anteriores queda detrás. Puro: no sabe de SDL ni de qué se dibuja.
class DepthOrder {
public:
	void clear();

	void addElement(int baseRow, std::uint32_t index);

	void addEntity(int row, std::uint32_t index);

	// ¿Hay elementos para intercalar? Sin ellos el orden de las
	// entidades no se toca (el dibujo de siempre).
	bool hasElements() const;

	// Ordenado por fila (estable: a igual fila y tipo, el orden en que
	// se agregaron).
	const std::vector<DepthItem>& sorted();

private:
	std::vector<DepthItem> items_;
	bool hasElements_ = false;
	bool sortedNow_ = false;
};

}
