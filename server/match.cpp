#include "match.h"

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <variant>

#include "commands/commands.h"
#include "domain/enemies/enemy_controller.h"
#include "domain/furniture/bed.h"
#include "domain/minerals/mine_controller.h"

// TODO: sistema de salas de la caverna: con sala 1 el oro no se genera
// (ver MineController::make_rock).
#define kDungeonRooms 10

// Un día de juego dura 14 min reales (840 s). El timeout del sueño sale de
// esta misma constante: cambiarlos juntos.
#define kDayDurationSeconds 840
// Hora de juego: de las 6:00 a las 2:00 del día siguiente (20 h), en saltos de 10 min.
// (kDayClockMinutes vive en common/common_constants.h)
#define kClockStepMinutes 10
#define kClockStepMs 100
// Pasos de kClockStepMs entre el reloj lento de criaturas (40 = 4 s). Ajustable.
#define kCreatureTickSteps 40
// Pasos de kClockStepMs entre persecución de enemigos y vuelo de llamas
// (4 = 400 ms, el mismo ritmo que da un paso el jugador).
#define kEnemyTickSteps 4

DayClock::DayClock(Queue<MatchEvent>& events):
        events(events), restart_requested(false), day_generation(0), start_minutes_(0) {}

void DayClock::restart(uint16_t start_minutes) {
    // Primero la base de tiempo, luego la generación: los pasos del día viejo
    // ya no valen.
    this->start_minutes_.store(start_minutes);
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
        uint8_t enemy_steps = 0;
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
            const auto elapsed_minutes = elapsed_ms * kDayClockMinutes / (kDayDurationSeconds * 1000) /
                                         kClockStepMinutes * kClockStepMinutes;
            uint16_t minutes = static_cast<uint16_t>(this->start_minutes_.load()) + static_cast<uint16_t>(elapsed_minutes);
            if (minutes > kDayClockMinutes) {
                minutes = kDayClockMinutes;
            }
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
            // Persecución y llamas: 10 veces más seguido que el reloj lento.
            if (++enemy_steps >= kEnemyTickSteps) {
                enemy_steps = 0;
                try {
                    events.push(MatchEvent(EnemyTick{}));
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
    for (uint8_t room = 1; room <= kDungeonRooms; ++room) {
        const DungeonFloor& floor = world.get_dungeon_floor(room);
        world.set_active_scene(floor.get_scene_id());
        MineController mine(room);
        mine.populate(world, floor.get_spawn_x(), floor.get_spawn_y());
    }
}

void Match::place_enemies() {
    for (uint8_t room = 1; room <= kDungeonRooms; ++room) {
        const DungeonFloor& floor = world.get_dungeon_floor(room);
        world.set_active_scene(floor.get_scene_id());
        EnemyController enemies(room);
        enemies.populate(world, floor.get_spawn_x(), floor.get_spawn_y());
    }
}

Match::Match(Queue<MatchEvent>& events, ServerOutputMonitor& output,
            worldmodel::World&& world_model):
        events(events),
        output(output),
        world(std::move(world_model)),
        clock(events),
        sleeping_sent(0),
        total_sent(0),
        clock_minutes_sent(0) {
    // Cabañas libres para asignar a los jugadores que entren.
    for (uint8_t k = 1; k <= world.cabin_count(); ++k) {
        free_cabins_.insert(k);
    }
    if (free_cabins_.empty()) {
        free_cabins_.insert(1);
    }
    // Inicio: las camas y los portales ya vienen del mundo (TOML).
    // Pueblo
    // Comerciantes nacen solos cuando se crea Town
    // TODO: Dejo nota para persistencia
    // Mazmorra
    place_minerals();
    place_enemies();
}

// Pasaron los 30 s sin que nadie durmiera: se termina el día. Todos duermen
// donde están y amanecen en la cama cuando despiertan.
void Match::handle(const DayPassed& /*day*/) {
    world.mark_all_fainted_by_timeout();
    world.end_day();
    flush_community_events();
    sync_day();
    push_sleep_state();
    sync_all();
}

void Match::handle(const CreatureTick& /*tick*/) {
    if (!world.is_waking()) {
        world.tick_animals();
        world.tick_enemies();
    }
    flush_community_events();
    sync_all();
}

void Match::handle(const EnemyTick& /*tick*/) {
    if (world.is_waking()) {
        flush_community_events();
        return;
    }
    // Solo sincroniza si algo cambió: hay llamas en vuelo o alguien persiguiendo.
    const bool flames_flying = world.tick_flames();
    const bool someone_pursuing = world.tick_enemies_pursuit();
    // Cuando vence la espera del muerto va a la cama (y queda en
    // sleepers): hay que mandar el estado aunque no haya enemigos ni
    // llamas en vuelo.
    const bool arrived_at_bed = world.tick_death_to_bed();
    // La muerte por daño (enemigo o llama) nace en este tick: sin empujar
    // estos estados el muerto se entera recién cuando manda un comando,
    // es decir, recién cuando se mueve. Los dos deduplican solos.
    sync_day();
    push_sleep_state();
    flush_community_events();
    if (flames_flying || someone_pursuing || arrived_at_bed) {
        sync_all();
    }
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
        // La caverna se regenera al comenzar el día, después de su limpieza
        // (DungeonFloor::on_new_day), tanto al dormir como por timeout.
        broadcast(ServerDayEnd{});
    }
    if (world.take_day_started()) {
        for (const uint8_t id: world.get_player_ids()) {
            Player& p = world.get_player(id);
            world.transfer_player(id, p.get_home_scene(), p.get_home_x(), p.get_home_y());
            world.reset_player_for_day(id);
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
    character_names[joined.player_id] = joined.characterName;
    uint8_t cabin_k = 1;
    if (!free_cabins_.empty()) {
        cabin_k = *free_cabins_.begin();
        free_cabins_.erase(free_cabins_.begin());
    }
    player_cabin_[joined.player_id] = cabin_k;
    auto bed = world.cabin_bed(cabin_k);
    world.add_player(joined.player_id,
                     std::make_unique<Player>(joined.player_id, bed.second.first,
                                              bed.second.second, bed.first,
                                              bed.first, bed.second.first,
                                              bed.second.second));
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
    if (std::holds_alternative<protocol::ChatMessage>(event.message)) {
        handle_chat(event.player_id,
                    std::get<protocol::ChatMessage>(event.message).text);
    }
    // El evento nace en el pickup (ver DroppedItem::interact), no de comparar
    // inventarios: solo se traduce a mensaje acá.
    for (const PlayerPickup& pickup: world.take_pickups()) {
        send_to(pickup.player_id, ServerObtained{content_id_for(pickup.item)});
    }
    for (const ResourceHit& hit: world.take_resource_hits()) {
        broadcast(ServerResourceHit{hit.x, hit.y, hit.destroyed});
    }
    for (const GiftOffer& offer : world.take_gift_requests()) {
        // El nombre va en el mensaje: el cliente no conoce a los jugadores
        // (mismo dato que usa el chat). Si el emisor ya no está en la sala,
        // se manda vacío y el copy cae en el token sin nombre.
        const auto sender = character_names.find(offer.sender_id);
        const std::string characterName =
                sender != character_names.end() ? sender->second : std::string{};
        send_to(offer.receiver_id,
                ServerGiftRequest{offer.sender_id, characterName, content_id_for(offer.item)});
    }
    // El emisor sigue esperando con el popup abierto: la respuesta del
    // receptor (acepte o rechace) se lo cierra.
    for (const uint8_t answered : world.take_gift_answers()) {
        send_to(answered, ServerGiftAnswered{});
    }
    for (const uint8_t receiver: world.take_accepted_gifts()) {
        broadcast(ServerGiftAccepted{receiver});
    }
    flush_community_events();
    sync_day();
    if (const auto desired = world.take_desired_clock_minutes()) {
        broadcast_clock(*desired);
    }
    if (const auto restart = world.take_clock_restart_request()) {
        clock.restart(*restart);
    }
    push_sleep_state();
    sync_all();
}

void Match::flush_community_events() {
    for (const uint8_t set_id: world.take_completed_sets()) {
        broadcast(ServerSetCompleted{set_id});
    }
    if (world.take_victory()) {
        broadcast(ServerVictory{});
        broadcast(ServerChat{0, "", "El Centro Comunitario fue completado", true});
    }
}

void Match::handle_chat(uint8_t player_id, const std::string& text) {
    for (const ChatReply& reply : chat_router.route(text, player_id, world, character_names)) {
        if (reply.broadcast) {
            broadcast(reply.message);
        } else {
            send_to(reply.target, reply.message);
        }
    }
}

void Match::handle(const PlayerLeft& left) {
    const auto it = outputs.find(left.player_id);
    if (it == outputs.end()) {
        return;
    }
    const std::shared_ptr<Queue<ServerMessage>> leaving = it->second;
    outputs.erase(it);
    character_names.erase(left.player_id);
    const auto cabinIt = player_cabin_.find(left.player_id);
    if (cabinIt != player_cabin_.end()) {
        free_cabins_.insert(cabinIt->second);
        player_cabin_.erase(cabinIt);
    }
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
