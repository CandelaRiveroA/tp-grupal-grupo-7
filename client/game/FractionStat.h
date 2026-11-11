#pragma once

namespace game {

// Identidad de un stat del jugador. Es vocabulario del juego (enum, no
// string): el estado no depende de las claves de presentación del HUD.
enum class StatKind {
	Energy,
	Life,
};

// Stat del jugador como fracción normalizada 0..1.
//
// Hoy lo simula el cliente (igual que el resto del "server falso"): las
// teclas de acción llaman a spend(). Mañana el server mandará el valor (o
// el gasto) y la presentación no cambia: solo cambia de dónde sale la
// fracción.
class FractionStat {
public:
	// Gasta `cost`, clampeando a 0.
	void spend(double cost);

	// Vuelve a lleno (comienzo de un día nuevo).
	void reset();

	// Fracción actual, 0.0 (vacía) a 1.0 (llena).
	double fraction() const;

private:
	double fraction_ = 1.0;
};

}
