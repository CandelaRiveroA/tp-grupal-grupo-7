#ifndef SERVER_COMMANDS_CHAT_COMMAND_H
#define SERVER_COMMANDS_CHAT_COMMAND_H

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "common/common_server_status.h"
#include "common/common_command.h"
#include "domain/map/world.h"
#include "commands/cmd/player_commands.h"
#include "commands/cmd/item_commands.h"
#include "commands/cmd/community_commands.h"
#include "commands/cmd/merchant_commands.h"

// Entrega a realizar sobre la cola de un jugador (o broadcast a todos).
struct ChatReply {
    bool broadcast;
    uint8_t target;
    ServerMessage message;
};

// Parsea y resuelve un mensaje de chat: texto libre (broadcast), privado
// (@nombre) o comando que empieza con '/'. Muta `world` si es un cheat.
// Devuelve las entregas a realizar (broadcast o send_to por jugador).
class ChatRouter {
public:
    ChatRouter();

    std::vector<ChatReply> route(const std::string& text, uint8_t sender,
                                 World& world,
                                 const std::map<uint8_t, std::string>& names);

private:
    using Handler = std::function<std::string(World&, uint8_t,
                                              const std::vector<std::string>&)>;
    std::map<std::string, Handler> handlers_;
    commands::PlayerCommands player_commands_;
    commands::ItemCommands item_commands_;
    commands::CommunityCommands community_commands_;
    commands::MerchantCommands merchant_commands_;
    std::vector<ChatReply> route_slash(const std::string& text, uint8_t sender,
                                       World& world);
    std::vector<ChatReply> route_private(const std::string& text, uint8_t sender,
                                         const std::string& sender_name,
                                         const std::map<uint8_t, std::string>& names);
    void add_merchant_offer(World& world, uint8_t sender,
                            std::vector<ChatReply>& replies) const;

    static std::vector<std::string> split(const std::string& s);
    static std::string trim(const std::string& s);
};

#endif  // SERVER_COMMANDS_CHAT_COMMAND_H