#ifndef GAMECLOCK_H
#define GAMECLOCK_H

#include <cstdint>

// Estructura ligera que el GameLoop tomará para armar el DTO de red
struct TimeInfoDTO {
    uint32_t days_passed;
    uint8_t hours;
    uint8_t minutes; // Garantizado de ser siempre 0, 10, 20, 30, 40 o 50
};

class GameClock {
private:
    uint32_t days_passed;
    uint16_t in_game_minutes;
    uint8_t tick_counter;
    bool day_ended;

public:
    // Por defecto arranca en el Día 1, pero puede recibir el día guardado en la base de datos
    GameClock(uint32_t initial_days = 1);
    ~GameClock() = default;

    GameClock(const GameClock&) = delete;
    GameClock& operator=(const GameClock&) = delete;

    // Avanza el reloj interno. Devuelve 'true' en el instante exacto en que dan las 2:00 AM.
    bool tick();
    
    // Resetea a las 6:00 AM y suma 1 día.
    void reset_for_new_day();

    // Empaqueta el tiempo traducido a horas y minutos en intervalos de 10m para la UI del cliente.
    TimeInfoDTO get_time_info() const;
    
    bool is_day_ended() const;
};

#endif // GAMECLOCK_H
