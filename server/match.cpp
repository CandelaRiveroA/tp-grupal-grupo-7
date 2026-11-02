#include "match.h"

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>

#include "commands/commands.h"
#include "domain/animals/chicken.h"
#include "domain/animals/cow.h"
#include "domain/furniture/bed.h"
#include "domain/minerals/coal_rock.h"
#include "domain/minerals/gold_rock.h"
#include "domain/minerals/lapis_rock.h"
#include "domain/minerals/silver_rock.h"

// Mismo tamaño que la habitación de client/sdl/assets/scenes.toml (bedroom).
#define kWidth 30
#define kHeight 20
#define kSpawnX 15
#define kSpawnY 10

// shortcut: un día fijo de 30s.
#define kDayDurationSeconds 30
#define kClockStepMs 100

DayClock::DayClock(Queue<MatchEvent>& events): events(events), restart_requested(false) {}

void DayClock::restart() { this->restart_requested.store(true); }

void DayClock::execute() {
    while (true) {
        // Un reinicio que llega justo al terminar el día ya no hace falta:
        // el día siguiente arranca igual.
        this->restart_requested.store(false);
        const auto day_start = std::chrono::steady_clock::now();
        bool interrupted = false;
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
    // Piedras de prueba alrededor del punto de aparición. Hasta que haya
    // generación de mundo, el servidor fija estas.
    world.set_entity_at(kSpawnX + 1, kSpawnY, std::make_unique<SilverRock>(kSpawnX + 1, kSpawnY));
    world.set_entity_at(kSpawnX + 2, kSpawnY, std::make_unique<CoalRock>(kSpawnX + 2, kSpawnY));
    world.set_entity_at(kSpawnX + 1, kSpawnY + 1,
                        std::make_unique<LapisRock>(kSpawnX + 1, kSpawnY + 1));
    world.set_entity_at(kSpawnX + 2, kSpawnY + 1,
                        std::make_unique<GoldRock>(kSpawnX + 2, kSpawnY + 1));
    world.set_entity_at(kSpawnX - 1, kSpawnY, std::make_unique<CoalRock>(kSpawnX - 1, kSpawnY));
    world.set_entity_at(kSpawnX, kSpawnY + 2, std::make_unique<SilverRock>(kSpawnX, kSpawnY + 2));
    world.set_entity_at(kSpawnX - 1, kSpawnY + 2,
                        std::make_unique<GoldRock>(kSpawnX - 1, kSpawnY + 2));
    world.set_entity_at(kSpawnX + 3, kSpawnY, std::make_unique<LapisRock>(kSpawnX + 3, kSpawnY));
}

void Match::place_animals() {
    // Animales de prueba, igual que las piedras: hasta que haya generación de mundo.
    world.add_animal(std::make_unique<Chicken>(3, 3));
    world.add_animal(std::make_unique<Chicken>(26, 4));
    world.add_cow(std::make_unique<Cow>(4, 4));
    world.add_cow(std::make_unique<Cow>(25, 5));
}

// Cama propia de cada jugador: fila de arriba del spawn, una por ID.
static uint16_t bed_x_for(uint8_t player_id) { return kSpawnX - 3 + (player_id - 1); }

Match::Match(Queue<MatchEvent>& events, ServerOutputMonitor& output):
        events(events),
        output(output),
        world(kWidth, kHeight),
        clock(events),
        sleeping_sent(0),
        total_sent(0) {
    place_minerals();
    place_animals();
}

// Pasaron los 30 s sin que nadie durmiera: todos duermen donde están y
// amanecen en su cama. Se cancela la votación abierta.
void Match::handle(const DayPassed& /*day*/) {
    world.clear_sleep();
    world.tick_day();
    for (const uint8_t id: world.get_player_ids()) {
        world.set_player_position(id, bed_x_for(id), kSpawnY - 1);
    }
    push_sleep_state();
    sync_all();
}

void Match::handle(const PlayerJoined& joined) {
    outputs[joined.player_id] = joined.statuses;
    world.set_entity_at(bed_x_for(joined.player_id), kSpawnY - 1,
                        std::make_unique<Bed>(bed_x_for(joined.player_id), kSpawnY - 1));
    world.add_player(joined.player_id,
                     std::make_unique<Player>(joined.player_id, kSpawnX, kSpawnY));
    // Sube el total de la votación: mientras no vaya a la cama, el sueño
    // no puede completarse.
    push_sleep_state();
    sync_all();
}

void Match::handle(const PlayerMessage& event) {
    std::visit([this,
                &event](const auto& message) { commands::apply(world, event.player_id, message); },
               event.message);
    if (std::holds_alternative<protocol::PickMessage>(event.message)) {
        broadcast(ServerToolUsed{event.player_id});
    }
    // El evento nace en el pickup (ver DroppedItem::interact), no de comparar
    // inventarios: solo se traduce a mensaje acá.
    for (const PlayerPickup& pickup: world.take_pickups()) {
        send_to(pickup.player_id, ServerObtained{content_id_for(pickup.item)});
    }
    for (const ResourceHit& hit: world.take_resource_hits()) {
        broadcast(ServerResourceHit{hit.x, hit.y, hit.destroyed});
    }
    // Dormir avanzó el día antes de tiempo: el tick ya corrió (los animales
    // produjeron), solo queda reiniciar el reloj.
    if (world.take_sleep_advanced()) {
        clock.restart();
    }
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
    if (world.take_sleep_advanced()) {
        clock.restart();
    }
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
    if (sleeping == this->sleeping_sent && total == this->total_sent) {
        return;
    }
    this->sleeping_sent = sleeping;
    this->total_sent = total;
    for (const uint8_t id: players) {
        send_to(id, ServerSleep{world.is_sleeping(id) ? sleeping : uint8_t{0}, total});
    }
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
