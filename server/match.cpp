#include "match.h"

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <variant>

#include "commands/commands.h"
#include "domain/animals/chicken.h"
#include "domain/animals/cow.h"
#include "domain/enemies/skeleton.h"
#include "domain/enemies/skull.h"
#include "domain/furniture/bed.h"
#include "domain/minerals/mine_controller.h"

// Mismo tamaño que la habitación de client/sdl/assets/scenes.toml (bedroom).
#define kWidth 30
#define kHeight 20
#define kSpawnX 15
#define kSpawnY 10

// TODO: sistema de salas de la caverna: con sala 1 el oro no se genera
// (ver MineController::create_entity).
#define kRoomNumber 1

// Misma celda que enter.lie de la cama en scenes.toml (bedroom). Una sola cama
// compartida: los durmientes no colisionan, así todos caben en ella.
#define kBedX 20
#define kBedY 13

// shortcut: un día de juego de 30s para probar; en la versión final son 15 min
// (900 s). El timeout del sueño sale de esta misma constante: cambiarlos juntos.
#define kDayDurationSeconds 60
// Hora de juego: de las 6:00 a las 2:00 del día siguiente (20 h), en saltos de 10 min.
#define kDayClockMinutes 1200
#define kClockStepMinutes 10
#define kClockStepMs 100
// Pasos de kClockStepMs entre movimiento de animales y enemigos (10 = 1 s). Ajustable.
#define kCreatureTickSteps 40

DayClock::DayClock(Queue<MatchEvent>& events):
        events(events), restart_requested(false), day_generation(0) {}

void DayClock::restart() {
    // Primero la generación: los pasos del día viejo ya no valen.
    this->day_generation.fetch_add(1);
    this->restart_requested.store(true);
}

uint32_t DayClock::generation() const { return this->day_generation.load(); }

void DayClock::execute() {
    while (true) {
        // Un reinicio que llega justo al terminar el día ya no hace falta:
        // el día siguiente arranca igual.
        this->restart_requested.store(false);
        const uint32_t current_generation = this->day_generation.load();
        const auto day_start = std::chrono::steady_clock::now();
        bool interrupted = false;
        uint8_t creature_steps = 0;
        uint16_t clock_minutes = 0;
        // Duerme en pasos cortos para que stop() corte sin esperar el día entero.
        while (std::chrono::steady_clock::now() - day_start <
               std::chrono::seconds(kDayDurationSeconds)) {
            if (!should_keep_running()) {
                return;
            }
            if (this->restart_requested.exchange(false)) {
                // El sueño avanzó el día: se corta sin empujar DayPassed.
                interrupted = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kClockStepMs));
            const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                            std::chrono::steady_clock::now() - day_start)
                                            .count();
            const auto minutes = elapsed_ms * kDayClockMinutes / (kDayDurationSeconds * 1000) /
                                 kClockStepMinutes * kClockStepMinutes;
            if (minutes != clock_minutes) {
                clock_minutes = static_cast<uint16_t>(minutes);
                try {
                    events.push(MatchEvent(ClockStep{clock_minutes, current_generation}));
                } catch (const ClosedQueue&) {
                    return;
                }
            }
            // Usa la misma cola que el día: el mundo solo lo toca el hilo de Match.
            if (++creature_steps >= kCreatureTickSteps) {
                creature_steps = 0;
                try {
                    events.push(MatchEvent(CreatureTick{}));
                } catch (const ClosedQueue&) {
                    return;
                }
            }
        }
        if (interrupted) {
            continue;
        }
        try {
            events.push(MatchEvent(DayPassed{}));
        } catch (const ClosedQueue&) {
            return;
        }
    }
}

void Match::place_minerals() {
    // El controller se construye de nuevo en cada llamada: así la semilla
    // aleatoria cambia y cada generación dispone los minerales distinto.
    MineController mine(kRoomNumber);
    mine.populate(world, kSpawnX, kSpawnY);
}

void Match::place_animals() {
    // Animales de prueba, igual que las piedras: hasta que haya generación de mundo.
    world.add_animal(std::make_unique<Chicken>(3, 3));
    world.add_animal(std::make_unique<Chicken>(26, 4));
    world.add_animal(std::make_unique<Cow>(4, 4));
    world.add_animal(std::make_unique<Cow>(25, 5));
}

void Match::place_enemies() {
    // Enemigos de prueba, igual que los animales: hasta que haya generación
    // de mundo. Lejos del spawn para que no ataque nada al entrar.
    world.add_enemy(std::make_unique<Skeleton>(2, 16));
    world.add_enemy(std::make_unique<Skeleton>(27, 17));
    // Manada de orcos de prueba (3 a 6), lejos del spawn (15, 10).
    world.spawn_orc_pack(15, 4);
    // Pareja de vampiros de prueba, lejos del spawn y de los orcos.
    world.spawn_vampire_pair(24, 12);
    // Un solo Skull: hoy hay un solo cuarto; el EnemyController repartirá uno por cuarto.
    world.add_enemy(std::make_unique<Skull>(5, 3));
}

Match::Match(Queue<MatchEvent>& events, ServerOutputMonitor& output):
        events(events),
        output(output),
        world(kWidth, kHeight),
        clock(events),
        sleeping_sent(0),
        total_sent(0),
        clock_minutes_sent(0) {
    place_animals();
    place_enemies();
    world.set_entity_at(kBedX, kBedY, std::make_unique<Bed>(kBedX, kBedY));
    place_minerals();
}

// Pasaron los 30 s sin que nadie durmiera: se termina el día. Todos duermen
// donde están y amanecen en la cama cuando despiertan.
void Match::handle(const DayPassed& /*day*/) {
    world.end_day();
    sync_day();
    push_sleep_state();
    sync_all();
}

void Match::handle(const CreatureTick& /*tick*/) {
    if (!world.is_waking()) {
        world.tick_animals();
        world.tick_enemies();
    }
    sync_all();
}

void Match::handle(const ClockStep& step) {
    // Un paso de un día reiniciado llega tarde: se descarta. Con el día
    // terminado el reloj queda congelado hasta que despierten todos.
    if (step.generation != clock.generation() || step.minutes == clock_minutes_sent ||
        world.is_waking()) {
        return;
    }
    broadcast_clock(step.minutes);
}

// Transiciones del día: al terminar, la pantalla se apaga; al arrancar, todos
// van a la cama y el reloj vuelve a las 6:00.
void Match::sync_day() {
    if (world.take_day_ended()) {
        // La caverna se repone en todo fin de día, sea por timeout
        // (DayPassed) o porque todos durmieron (World::try_advance_sleep),
        // que cierra el día sin pasar por DayPassed.
        place_minerals();
        broadcast(ServerDayEnd{});
    }
    if (world.take_day_started()) {
        for (const uint8_t id: world.get_player_ids()) {
            world.set_player_position(id, kBedX, kBedY);
        }
        clock.restart();
        broadcast(ServerDay{});
        broadcast_clock(0);
    }
}

void Match::broadcast_clock(uint16_t minutes) {
    this->clock_minutes_sent = minutes;
    broadcast(ServerClock{world.get_day(), minutes});
}

void Match::handle(const PlayerJoined& joined) {
    outputs[joined.player_id] = joined.statuses;
    world.add_player(joined.player_id,
                     std::make_unique<Player>(joined.player_id, kSpawnX, kSpawnY));
    world.set_player_appearance(joined.player_id, joined.appearance);
    // Si el día ya terminó, el que entra también tiene que despertar.
    if (world.is_waking()) {
        send_to(joined.player_id, ServerDayEnd{});
    }
    // Sube el total de la votación: mientras no vaya a la cama, el sueño
    // no puede completarse.
    push_sleep_state();
    send_to(joined.player_id, ServerClock{world.get_day(), clock_minutes_sent});
    sync_all();
}

void Match::handle(const PlayerMessage& event) {
    std::visit([this,
                &event](const auto& message) { commands::apply(world, event.player_id, message); },
               event.message);
    if (std::holds_alternative<protocol::PickMessage>(event.message)) {
        const Item* equipped = world.get_player(event.player_id).peek_equipped_item();
        broadcast(ServerToolUsed{event.player_id,
                                 equipped ? content_id_for(equipped->get_id()) : ""});
    }
    // El evento nace en el pickup (ver DroppedItem::interact), no de comparar
    // inventarios: solo se traduce a mensaje acá.
    for (const PlayerPickup& pickup: world.take_pickups()) {
        send_to(pickup.player_id, ServerObtained{content_id_for(pickup.item)});
    }
    for (const ResourceHit& hit: world.take_resource_hits()) {
        broadcast(ServerResourceHit{hit.x, hit.y, hit.destroyed});
    }
    sync_day();
    push_sleep_state();
    sync_all();
}

void Match::handle(const PlayerLeft& left) {
    const auto it = outputs.find(left.player_id);
    if (it == outputs.end()) {
        return;
    }
    const std::shared_ptr<Queue<ServerMessage>> leaving = it->second;
    outputs.erase(it);
    // Saca al jugador de la votación: nadie queda esperando a un ausente.
    world.remove_player(left.player_id);
    sync_day();
    push_sleep_state();
    broadcast(ServerLeave{left.player_id});
    leaving->close();
}

void Match::closeOutputs() {
    for (auto& [id, statuses]: outputs) {
        try {
            statuses->close();
        } catch (const std::runtime_error&) {}
    }
    outputs.clear();
}

void Match::sync_all() {
    for (const auto& [id, statuses]: outputs) {
        for (const ServerMessage& message: views.sync(world, id)) {
            try {
                statuses->push(message);
            } catch (const ClosedQueue&) {
                // Un cliente desconectado no debe detener la partida para los demas.
            }
        }
    }
}

void Match::broadcast(const ServerMessage& message) {
    for (const auto& [id, statuses]: outputs) {
        try {
            statuses->push(message);
        } catch (const ClosedQueue&) {
            // Un cliente desconectado no debe detener la partida para los demas.
        }
    }
}

void Match::send_to(uint8_t player_id, const ServerMessage& message) {
    const auto it = outputs.find(player_id);
    if (it == outputs.end()) {
        return;
    }
    try {
        it->second->push(message);
    } catch (const ClosedQueue&) {
        // Un cliente desconectado no debe detener la partida para los demas.
    }
}

// El aviso de sueño solo lo ven los jugadores que están en la cama: los
// demás reciben 0 (no se les abre nada) y siguen jugando con el día
// corriendo. sleeping > 0 es el "X de N" que ven los durmientes.
void Match::push_sleep_state() {
    const uint8_t sleeping = world.sleeping_count();
    const std::vector<uint8_t> players = world.get_player_ids();
    const uint8_t total = static_cast<uint8_t>(players.size());
    std::vector<uint8_t> sleepers;
    for (const uint8_t id: players) {
        if (world.is_sleeping(id)) {
            // cppcheck-suppress useStlAlgorithm; copy_if con lambda queda más
            // ruido que este filtro de tres líneas.
            sleepers.push_back(id);
        }
    }
    if (sleeping == this->sleeping_sent && total == this->total_sent &&
        sleepers == this->sleepers_sent) {
        return;
    }
    this->sleeping_sent = sleeping;
    this->total_sent = total;
    this->sleepers_sent = sleepers;
    for (const uint8_t id: players) {
        send_to(id, ServerSleep{world.is_sleeping(id) ? sleeping : uint8_t{0}, total});
    }
    broadcast(ServerSleepingPlayers{sleepers});
}

void Match::execute() {
    clock.start();
    try {
        while (should_keep_running()) {
            const MatchEvent event = events.pop();
            std::visit([this](const auto& alternative) { handle(alternative); }, event);
        }
    } catch (const ClosedQueue&) {
        closeOutputs();
        clock.stop();
        clock.join();
        throw;
    } catch (const std::exception& error) {
        closeOutputs();
        events.close();
        if (should_keep_running()) {
            output.printError("Error en la partida: ", error.what());
        }
        clock.stop();
        clock.join();
        throw;
    }
    clock.stop();
    clock.join();
}
