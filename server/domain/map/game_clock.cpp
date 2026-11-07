#include "game_clock.h"

// 360 minutos desde las 00:00 representan las 6:00 AM.
GameClock::GameClock(uint32_t initial_days) 
    : days_passed(initial_days), 
      in_game_minutes(360), 
      real_time_accumulator(0),
      day_ended(false) {}

bool GameClock::update(std::chrono::milliseconds elapsed_real_time) {
    if (this->day_ended) return false;

    this->real_time_accumulator += elapsed_real_time;

    // Patrón Acumulador para lidiar con el lag del servidor
    while (this->real_time_accumulator >= kRealMsPerIngameMinute) {
        this->real_time_accumulator -= kRealMsPerIngameMinute;
        this->in_game_minutes++;

        if (this->in_game_minutes >= kEndOfDayMinutes) {
            this->day_ended = true;
            return true; // Dispara el trigger para procesar el fin de día en Match
        }
    }
    
    return false;
}

void GameClock::reset_for_new_day() {
    this->in_game_minutes = 360; // Volvemos a las 6:00 AM
    this->real_time_accumulator = std::chrono::milliseconds(0);
    this->day_ended = false;
}

void GameClock::set_days_passed(uint32_t current_day) {
    this->days_passed = current_day;
}

// ---------------------------------------------------------
// Generador del DTO
// ---------------------------------------------------------
TimeInfoDTO GameClock::get_time_info() const {
    TimeInfoDTO dto;
    
    // Matemática de días
    dto.days_passed = this->days_passed;
    
    // Si days_passed es 1, cae en el día 1 de la estación. 
    // Los meses tienen 28 días. Modulo 28. (Usamos -1 y +1 para que vaya de 1 a 28, no de 0 a 27)
    dto.day_of_season = ((this->days_passed - 1) % 28) + 1;
    
    // Si days_passed es 1, es el día 1 de la semana (Lunes).
    dto.day_of_week = ((this->days_passed - 1) % 7) + 1;
    
    // Matemática de horas (formato 24hs). 
    // Ej 6:00 AM -> 360 / 60 = 6. 
    // Ej 2:00 AM -> 1560 / 60 = 26. 26 % 24 = 2 (2 AM).
    dto.hours = (this->in_game_minutes / 60) % 24;
    
    // Matemática de minutos. Truncamos hacia el múltiplo de 10 inferior (0, 10, 20, 30, 40, 50).
    uint8_t exact_minutes = this->in_game_minutes % 60;
    dto.minutes = (exact_minutes / 10) * 10;

    return dto;
}

bool GameClock::is_day_ended() const {
    return this->day_ended;
}

uint16_t GameClock::get_in_game_minutes() const {
    return this->in_game_minutes;
}
