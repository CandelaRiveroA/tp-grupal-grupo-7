#include "chat_command.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "common/common_constants.h"
#include "domain/npc/merchant.h"

namespace commands {
void apply(World& /*world*/, uint8_t /*player_id*/, const protocol::ChatMessage& /*message*/) {
    // El chat no muta el mundo acá: el routing (broadcast/privado/cheats) lo
    // hace Match::handle_chat, que necesita acceso a broadcast/send_to.
    // Este overload existe porque std::visit en Match::handle lo instancia.
}
}  // namespace commands

ChatRouter::ChatRouter() {
    handlers_["/vida"] = [this](World& world, uint8_t sender,
                                const std::vector<std::string>& args) {
        return player_commands_.vida(world, sender, args);
    };
    handlers_["/energia"] = [this](World& world, uint8_t sender,
                                   const std::vector<std::string>& args) {
        return player_commands_.energia(world, sender, args);
    };
    handlers_["/oro"] = [this](World& world, uint8_t sender,
                               const std::vector<std::string>& args) {
        return player_commands_.oro(world, sender, args);
    };
    handlers_["/donar"] = [this](World& world, uint8_t sender,
                                 const std::vector<std::string>& args) {
        return community_commands_.donar(world, sender, args);
    };
    handlers_["/completar_set"] = [this](World& world, uint8_t sender,
                                         const std::vector<std::string>& args) {
        return community_commands_.completar_set(world, sender, args);
    };
    handlers_["/completar_todo"] = [this](World& world, uint8_t sender,
                                          const std::vector<std::string>& args) {
        return community_commands_.completar_todo(world, sender, args);
    };
    handlers_["/consultar_catalogo"] = [this](World& world, uint8_t sender,
                                              const std::vector<std::string>& args) {
        return merchant_commands_.consultar_catalogo(world, sender, args);
    };
    handlers_["/comprar"] = [this](World& world, uint8_t sender,
                                   const std::vector<std::string>& args) {
        return merchant_commands_.comprar(world, sender, args);
    };
    handlers_["/vender"] = [this](World& world, uint8_t sender,
                                  const std::vector<std::string>& args) {
        return merchant_commands_.vender(world, sender, args);
    };
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

std::vector<ChatReply> ChatRouter::route(const std::string& text, uint8_t sender,
                                         World& world,
                                         const std::map<uint8_t, std::string>& names) {
    const std::string t = trim(text);
    if (t.empty()) return {};
    if (static_cast<int>(t.size()) > kMaxChatLength) {
        return {{false, sender, ServerChat{0, "", "mensaje demasiado largo", true}}};
    }

    const auto sender_it = names.find(sender);
    const std::string sender_name = (sender_it != names.end()) ? sender_it->second : "";

    if (t[0] == '/') {
        return route_slash(t, sender, world);
    }

    if (t[0] == '@') {
        return route_private(t, sender, sender_name, names);
    }

    return {{true, 0, ServerChat{sender, sender_name, t, false}}};
}

std::vector<ChatReply> ChatRouter::route_slash(const std::string& text, uint8_t sender,
                                               World& world) {
    std::vector<ChatReply> replies;
    const auto toks = split(text);
    const std::string cmd = toks.empty() ? std::string() : toks[0];
    std::vector<std::string> args(toks.begin() + (toks.empty() ? 0 : 1), toks.end());

    const auto it = handlers_.find(cmd);
    if (it == handlers_.end()) {
        replies.push_back({false, sender,
                           ServerChat{0, "", "comando no reconocido: " + cmd, true}});
        return replies;
    }

    const std::string response = it->second(world, sender, args);
    if (!response.empty()) {
        replies.push_back({false, sender, ServerChat{0, "", response, true}});
    }
    if (cmd == "/consultar_catalogo") {
        add_merchant_offer(world, sender, replies);
    }
    return replies;
}

std::vector<ChatReply> ChatRouter::route_private(
        const std::string& text, uint8_t sender, const std::string& sender_name,
        const std::map<uint8_t, std::string>& names) {
    const auto space = text.find(' ');
    if (space == std::string::npos) {
        return {{false, sender,
                 ServerChat{0, "", "uso: @nombre mensaje", true}}};
    }

    const std::string name = text.substr(1, space - 1);
    const std::string message = trim(text.substr(space + 1));
    if (message.empty()) {
        return {{false, sender,
                 ServerChat{0, "", "mensaje vacio", true}}};
    }

    uint8_t target = 0;
    bool found = false;
    for (const auto& [id, name_in_map] : names) {
        if (name_in_map == name) {
            target = id;
            found = true;
            break;
        }
    }
    if (!found) {
        return {{false, sender,
                 ServerChat{0, "", "el jugador " + name + " no esta conectado", true}}};
    }

    std::vector<ChatReply> replies;
    const ServerChat chat{sender, sender_name, text, false};
    replies.push_back({false, target, chat});
    if (target != sender) {
        replies.push_back({false, sender, chat});
    }
    return replies;
}

void ChatRouter::add_merchant_offer(World& world, uint8_t sender,
                                    std::vector<ChatReply>& replies) const {
    const std::optional<ServerMarketOffer> offer =
            merchant_commands_.offer_for(world, sender);
    if (!offer.has_value()) {
        return;
    }
    replies.push_back({false, sender, *offer});
}

