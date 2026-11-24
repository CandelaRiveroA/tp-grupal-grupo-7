#include "commands/cmd/player_commands.h"

#include <algorithm>
#include <stdexcept>

#include "common/common_constants.h"

namespace commands {

bool PlayerCommands::parse_int(const std::string& text, int& value) const {
    try {
        std::size_t pos = 0;
        const int parsed = std::stoi(text, &pos);
        if (pos != text.size()) {
            return false;
        }
        value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string PlayerCommands::vida(World& world, uint8_t sender,
                                 const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /vida [cantidad]";
    int value = 0;
    if (!parse_int(args[0], value)) return "cantidad invalida";
    Player& player = world.get_player(sender);
    value = std::clamp(value, 0, static_cast<int>(player.get_max_health()));
    if (value == 0) {
        world.kill_player(sender);
    } else {
        player.set_health(static_cast<uint16_t>(value));
        player.setDiedInCombat(false);
    }
    return "vida: " + std::to_string(value);
}

std::string PlayerCommands::energia(World& world, uint8_t sender,
                                    const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /energia [cantidad]";
    int value = 0;
    if (!parse_int(args[0], value)) return "cantidad invalida";
    Player& player = world.get_player(sender);
    const int max_percent = static_cast<int>(player.get_max_energy() / 100);
    value = std::clamp(value, 0, max_percent);
    const uint16_t scaled = static_cast<uint16_t>(value * 100);
    player.set_energy(scaled);
    if (scaled == 0) {
        world.faint_player(sender);
    } else {
        player.setFaintedByEnergy(false);
    }
    return "energia: " + std::to_string(value);
}

std::string PlayerCommands::oro(World& world, uint8_t sender,
                                const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /oro [cantidad]";
    int delta = 0;
    if (!parse_int(args[0], delta)) return "cantidad invalida";
    Player& player = world.get_player(sender);
    delta = std::clamp(delta, -65535, 65535);
    const int nuevo = std::clamp(
            static_cast<int>(player.get_gold()) + delta, 0, 65535);
    player.set_gold(static_cast<uint16_t>(nuevo));
    return "oro: " + std::to_string(nuevo);
}

std::string PlayerCommands::terminar_dia(World& world, uint8_t sender,
                                         const std::vector<std::string>& args) {
    (void)sender;
    if (!args.empty()) {
        return "uso: /terminar_dia";
    }
    world.end_day();
    return "día terminado";
}

std::string PlayerCommands::tiempo(World& world, uint8_t sender,
                                   const std::vector<std::string>& args) {
    (void)sender;
    if (args.size() != 1) {
        return "uso: /tiempo [minutos]";
    }
    int value = 0;
    if (!parse_int(args[0], value)) {
        return "cantidad invalida";
    }
    value = std::clamp(value, 0, kDayClockMinutes);
    const uint16_t minutes = static_cast<uint16_t>(value);
    world.set_clock_minutes(minutes);
    world.request_clock_restart(minutes);
    return "hora: " + std::to_string(minutes);
}

std::string PlayerCommands::tp(World& world, uint8_t sender,
                               const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /tp [escenario]";
    const std::string& name = args[0];
    Scene* scene_ptr = nullptr;
    try {
        scene_ptr = &world.get_scene(name);
    } catch (const std::exception&) {
        return "escenario invalido: " + name;
    }
    uint16_t x = 0, y = 0;
    bool found = false;
    for (uint16_t yy = 0; yy < scene_ptr->get_height() && !found; ++yy) {
        for (uint16_t xx = 0; xx < scene_ptr->get_width(); ++xx) {
            if (scene_ptr->can_walk_to(xx, yy)) {
                x = xx;
                y = yy;
                found = true;
                break;
            }
        }
    }
    if (!found) return "escenario sin celdas caminables: " + name;
    world.transfer_player(sender, name, x, y);
    return "escenario: " + name;
}

std::string PlayerCommands::tp_info(World& world, uint8_t sender,
                                    const std::vector<std::string>& args) {
    (void)sender;
    if (!args.empty()) return "uso: /tp_info";
    const std::vector<SceneId> ids = world.get_scene_ids();
    std::string out = "escenarios:";
    for (std::size_t i = 0; i < ids.size(); ++i) {
        out += (i == 0 ? " " : ", ") + ids[i];
    }
    return out;
}

}  // namespace commands
