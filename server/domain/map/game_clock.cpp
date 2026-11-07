#include "game_clock.h"

// 360 minutos desde las 00:00 representan las 6:00 AM.
GameClock::GameClock(uint32_t initial_days) 
    : days_passed(initial_days), in_game_minutes(360), tick_counter(0), day_ended(false) {}

bool GameClock::tick() {
    // Si el día ya terminó, el reloj se congela hasta que se reinicie el día
    if (this->day_ended) return false;

    this->tick_counter++;
    
    // A 30 FPS[cite: 6]: 14 mins reales = 25.200 ticks.
    // 20 horas de juego = 1.200 minutos in-game[cite: 10].
    // 25.200 ticks totales / 1.200 minutos in-game = 21 ticks por minuto.
    if (this->tick_counter >= 21) {
        this->in_game_minutes++;
        this->tick_counter = 0;

        // 1560 minutos desde las 00:00 representan las 2:00 AM del día siguiente[cite: 10]
        if (this->in_game_minutes >= 1560) {
            this->day_ended = true;
            return true; // Dispara el trigger para procesar el fin de día en el GameLoop
        }
    }
    return false;
}

void GameClock::reset_for_new_day() {
    this->days_passed++;
    this->in_game_minutes = 360; // Volvemos a las 6:00 AM
    this->tick_counter = 0;
    this->day_ended = false;
}

TimeInfoDTO GameClock::get_time_info() const {
    TimeInfoDTO dto;
    dto.days_passed = this->days_passed;
    
    // Matemática de horas (formato 24hs). 
    // Ej 6:00 AM -> 360 / 60 = 6. 
    // Ej 2:00 AM -> 1560 / 60 = 26. 26 % 24 = 2 AM.
    dto.hours = (this->in_game_minutes / 60) % 24;
    
    // Obtenemos los minutos exactos actuales (0 a 59)
    uint8_t exact_minutes = this->in_game_minutes % 60;
    
    // Truncamos hacia el múltiplo de 10 inferior (0, 10, 20, 30, 40, 50).
    // La división entera descarta el remanente (ej. 37 / 10 = 3). Luego multiplicamos por 10 (3 * 10 = 30).
    dto.minutes = (exact_minutes / 10) * 10;

    return dto;
}

bool GameClock::is_day_ended() const {
    return this->day_ended;
}