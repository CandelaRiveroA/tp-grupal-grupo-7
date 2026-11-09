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

// ANDAMIAJE TEMPORAL: primer id de los enemigos del bot de prueba
// (los del mundo simulado de granja arrancan en 1). Se va con el bot
// cuando el server mande los enemigos.
constexpr int kEnemyBotFirstId = 1000;

// --- Reloj de juego (ServerClock) ---------------------------------------

// El server manda los minutos desde esta hora (0 = 6:00).
constexpr int kClockStartHour = 6;
constexpr int kMinutesPerHour = 60;
// El día 1 es lunes: el día de la semana es (día - 1) % 7, 0 = lunes.
constexpr int kFirstDayNumber = 1;
constexpr int kDaysPerWeek = 7;

// Claves de los valores crudos del reloj que leen las etiquetas del HUD
// (los {token} / `source` de ui/hud.toml).
constexpr const char* kClockWeekdayKey = "dia_semana";
constexpr const char* kClockDayKey = "dia";
constexpr const char* kClockHourKey = "hora";
constexpr const char* kClockMinuteKey = "minuto";

}
