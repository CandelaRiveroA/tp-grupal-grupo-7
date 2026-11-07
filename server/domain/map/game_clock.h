#ifndef GAMECLOCK_H
#define GAMECLOCK_H

#include <cstdint>
#include <chrono>

// Estructura lista para enviar por la red (DTO ServerClock).
// El cliente no tiene que calcular absolutamente nada.
struct TimeInfoDTO {
    uint32_t days_passed; 
    uint8_t hours;         // Formato 24h (Ej: 6, 14, 23, 1, 2)
    uint8_t minutes;       // Garantizado de ser 0, 10, 20, 30, 40 o 50
};

class GameClock {
private:
    uint32_t days_passed;
    uint16_t in_game_minutes;
    std::chrono::milliseconds real_time_accumulator;
    bool day_ended;
    
    // Matemática del tiempo:
    // 14 minutos reales = 840,000 ms.
    // 20 horas in-game = 1,200 minutos in-game.
    // 840,000 ms / 1,200 min = 700 ms reales por cada minuto in-game.
    static constexpr std::chrono::milliseconds kRealMsPerIngameMinute{700};
    // 1560 minutos desde las 00:00 representan las 2:00 AM.
    static constexpr uint16_t kEndOfDayMinutes = 1560;

public:
    explicit GameClock(uint32_t initial_days = 1);
    ~GameClock() = default;

    GameClock(const GameClock&) = delete;
    GameClock& operator=(const GameClock&) = delete;

    // Avanza el reloj interno según el tiempo real transcurrido desde la última llamada.
    bool update(std::chrono::milliseconds elapsed_real_time);
    
    // Resetea el estado a las 6:00 AM y vacía el acumulador de tiempo.
    void reset_for_new_day();

    // Empaqueta el tiempo traducido a días, horas y minutos exactos para la UI.
    TimeInfoDTO get_time_info() const;
    
    void set_days_passed(uint32_t current_day);
    
    bool is_day_ended() const;
    uint16_t get_in_game_minutes() const;
};

#endif // GAMECLOCK_H