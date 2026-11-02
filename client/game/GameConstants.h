#pragma once

#include <cstddef>

namespace game {

// Constantes de la capa de estado/vista del juego.
constexpr std::size_t kLocalPlayerIndex = 0;

// --- Reglas del "server falso" (el server real pondrá las suyas) -------

// Tope de ítems en el piso del mundo: sin recogida el mundo se llena.
constexpr int kMaxGroundItems = 3;

// Vueltas de la animación por interacción: la leche da N ciclos (hoy lo
// fija esta capa; mañana el server, "hacelo N veces") y recoger es una
// sola pasada.
constexpr int kDefaultMilkCycles = 2;
constexpr int kPickupCycles = 1;

// Semilla de la simulación de animales: determinista (testeable).
constexpr unsigned int kAnimalSeed = 20261027;

}
