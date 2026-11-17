#include "chat_command.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "common/common_constants.h"

namespace commands {
void apply(World& /*world*/, uint8_t /*player_id*/, const protocol::ChatMessage& /*message*/) {
    // El chat no muta el mundo acá: el routing (broadcast/privado/cheats) lo
    // hace Match::handle_chat, que necesita acceso a broadcast/send_to.
    // Este overload existe porque std::visit en Match::handle lo instancia.
}
}  // namespace commands

ChatRouter::ChatRouter() {
    handlers_["/vida"] = &ChatRouter::cmd_vida;
    handlers_["/energia"] = &ChatRouter::cmd_energia;
    handlers_["/oro"] = &ChatRouter::cmd_oro;
}

std::vector<std::string> ChatRouter::split(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

std::string ChatRouter::trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool ChatRouter::parse_int(const std::string& s, int& out) {
    try {
        std::size_t pos = 0;
        const int value = std::stoi(s, &pos);
        if (pos != s.size()) return false;
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool ChatRouter::parse_float(const std::string& s, float& out) {
    try {
        std::size_t pos = 0;
        const float value = std::stof(s, &pos);
        if (pos != s.size() || !std::isfinite(value)) return false;
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<ChatReply> ChatRouter::route(const std::string& text, uint8_t sender,
                                         World& world,
                                         const std::map<uint8_t, std::string>& names) {
    std::vector<ChatReply> replies;
    const std::string t = trim(text);
    if (t.empty()) return replies;
    if (static_cast<int>(t.size()) > kMaxChatLength) {
        replies.push_back({false, sender, ServerChat{0, "", "mensaje demasiado largo", true}});
        return replies;
    }

    const auto sender_it = names.find(sender);
    const std::string sender_name = (sender_it != names.end()) ? sender_it->second : "";

    if (t[0] == '/') {
        const auto toks = split(t);
        const std::string cmd = toks.empty() ? std::string() : toks[0];
        std::vector<std::string> args(toks.begin() + (toks.empty() ? 0 : 1), toks.end());

        const auto it = handlers_.find(cmd);
        if (it == handlers_.end()) {
            replies.push_back({false, sender,
                               ServerChat{0, "", "comando no reconocido: " + cmd, true}});
            return replies;
        }
        std::string response = (this->*(it->second))(world, sender, args);
        if (!response.empty()) {
            replies.push_back({false, sender, ServerChat{0, "", response, true}});
        }
        return replies;
    }

    if (t[0] == '@') {
        const auto space = t.find(' ');
        if (space == std::string::npos) {
            replies.push_back({false, sender,
                               ServerChat{0, "", "uso: @nombre mensaje", true}});
            return replies;
        }
        const std::string name = t.substr(1, space - 1);
        const std::string msg = trim(t.substr(space + 1));
        if (msg.empty()) {
            replies.push_back({false, sender,
                               ServerChat{0, "", "mensaje vacio", true}});
            return replies;
        }
        uint8_t target = 0;
        bool found = false;
        for (const auto& [id, n] : names) {
            if (n == name) { target = id; found = true; break; }
        }
        if (!found) {
            replies.push_back({false, sender,
                               ServerChat{0, "", "el jugador " + name + " no esta conectado", true}});
            return replies;
        }
        const ServerChat chat{sender, sender_name, t, false};
        replies.push_back({false, target, chat});
        if (target != sender) {
            replies.push_back({false, sender, chat});
        }
        return replies;
    }

    // Mensaje público: broadcast.
    replies.push_back({true, 0, ServerChat{sender, sender_name, t, false}});
    return replies;
}

std::string ChatRouter::cmd_vida(World& world, uint8_t sender,
                                 const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /vida [cantidad]";
    int value = 0;
    if (!parse_int(args[0], value)) return "cantidad invalida";
    Player& p = world.get_player(sender);
    value = std::clamp(value, 0, static_cast<int>(p.get_max_health()));
    p.set_health(static_cast<uint16_t>(value));
    return "vida: " + std::to_string(value);
}

std::string ChatRouter::cmd_energia(World& world, uint8_t sender,
                                    const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /energia [cantidad]";
    float value = 0.0f;
    if (!parse_float(args[0], value)) return "cantidad invalida";
    Player& p = world.get_player(sender);
    value = std::clamp(value, 0.0f, p.get_max_energy());
    p.set_energy(value);
    return "energia: " + std::to_string(static_cast<int>(value));
}

std::string ChatRouter::cmd_oro(World& world, uint8_t sender,
                                const std::vector<std::string>& args) {
    if (args.size() != 1) return "uso: /oro [cantidad]";
    int delta = 0;
    if (!parse_int(args[0], delta)) return "cantidad invalida";
    Player& p = world.get_player(sender);
    // Acota el delta antes de sumar: sin esto la suma puede desbordar un int.
    delta = std::clamp(delta, -65535, 65535);
    int nuevo = std::clamp(static_cast<int>(p.get_gold()) + delta, 0, 65535);
    p.set_gold(static_cast<uint16_t>(nuevo));
    return "oro: " + std::to_string(nuevo);
}