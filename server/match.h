#ifndef MATCH_H
#define MATCH_H

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <variant>
#include <vector>

#include "domain/map/world.h"

#include "common_command.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_worker.h"
#include "server_output_monitor.h"
#include "world_view.h"

struct PlayerMessage {
    uint8_t player_id;
    protocol::ClientMessage message;
};

struct PlayerJoined {
    uint8_t player_id;
    std::shared_ptr<Queue<ServerMessage>> statuses;
};

struct PlayerLeft {
    uint8_t player_id;
};

struct DayPassed {};

// Cambio de salto de la hora de juego. `generation` identifica el día que lo
// produjo: un reinicio del día (sueño, salida) deja pasos viejos en la cola
// y Match los descarta.
struct ClockStep {
    uint16_t minutes;
    uint32_t generation;
};

// Dispara el paso de movimiento de los animales (un paso por segundo).
struct AnimalTick {};

using MatchEvent =
        std::variant<PlayerJoined, PlayerMessage, PlayerLeft, DayPassed, AnimalTick, ClockStep>;

// Empuja un DayPassed cada día de juego mientras la partida corre.
// restart() corta el día en curso SIN empujar DayPassed: lo usa el sueño,
// que ya avanzó el día, para que los 30 segundos arranquen de cero.
class DayClock: public Worker {
private:
    Queue<MatchEvent>& events;
    std::atomic<bool> restart_requested;
    std::atomic<uint32_t> day_generation;

public:
    explicit DayClock(Queue<MatchEvent>& events);

    void restart();
    uint32_t generation() const;

protected:
    void execute() override;
};

class Match: public Worker {
private:
    Queue<MatchEvent>& events;
    ServerOutputMonitor& output;
    World world;
    DayClock clock;
    std::map<uint8_t, std::shared_ptr<Queue<ServerMessage>>> outputs;
    WorldView views;
    // Último ServerSleep enviado: solo se manda si cambió.
    uint8_t sleeping_sent;
    uint8_t total_sent;
    std::vector<uint8_t> sleepers_sent;
    // Última hora de juego enviada (minutos desde las 6:00).
    uint16_t clock_minutes_sent;

    void handle(const PlayerJoined& joined);
    void handle(const PlayerMessage& event);
    void handle(const PlayerLeft& left);
    void handle(const DayPassed& day);
    void handle(const AnimalTick& tick);
    void handle(const ClockStep& step);
    void sync_all();
    void place_minerals();
    void place_animals();
    void broadcast(const ServerMessage& message);
    void send_to(uint8_t player_id, const ServerMessage& message);
    void push_sleep_state();
    void broadcast_clock(uint16_t minutes);
    void closeOutputs();

public:
    Match(Queue<MatchEvent>& events, ServerOutputMonitor& output);

protected:
    void execute() override;
};

#endif
