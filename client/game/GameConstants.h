#pragma once

#include <cstddef>

namespace game {

// Constantes de la capa de estado/vista del juego.
constexpr std::size_t kLocalPlayerIndex = 0;

// Ids de aviso (ViewNotice::id); cada uno tiene su texto en ui/notice.toml.
constexpr const char* kNoticeInventoryFull = "inventory_full";

// Celdas de alcance alrededor de lo que se toca (la cama, un edificio):
// el click solo hace algo si el jugador está sobre el elemento o a esta
// distancia.
constexpr int kInteractRing = 1;

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

// ANDAMIAJE TEMPORAL: primer id de los proyectiles del bot (arranca
// lejos de enemigos y animales: ViewMotion enlaza sus tracks por id).
constexpr int kEnemyProjectileFirstId = 2000;

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

// --- Stats simulados (energía y vida) ----------------------------------

// Claves de los valores 0..1 que llenan las barras del HUD
// (`[overlay.fill].value` en ui/hud.toml).
constexpr const char* kHudEnergyKey = "energia";
constexpr const char* kHudLifeKey = "vida";

// --- Plata del jugador -------------------------------------------------

// Plata con la que se muestra el HUD hasta que el server mande la suya
// (el mismo valor que el default de SaveSummary::gold).
constexpr int kStartingGold = 500;

// Clave del valor crudo que lee la etiqueta de plata del HUD ({plata} en
// ui/hud.toml).
constexpr const char* kHudGoldKey = "plata";

// --- Chat --------------------------------------------------------------

// Mensajes que guarda el historial; el más viejo se descarta.
constexpr std::size_t kChatHistoryMax = 10;

// Nombre del jugador local: el lobby todavía no lo pasa al cliente.
constexpr const char* kLocalPlayerName = "Vos";

}
