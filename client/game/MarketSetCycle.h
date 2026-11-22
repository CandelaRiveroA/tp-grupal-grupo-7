#pragma once

namespace game {

// PROVISORIA: el estilo (set) del catálogo que se muestra. Desaparece cuando
// el servidor rote los estilos y mande la oferta ya resuelta.
class MarketSetCycle {
public:
	int current() const;

	// Avanza cíclicamente entre `setCount` estilos (mínimo 1).
	void next(int setCount);

private:
	int current_ = 0;
};

}
