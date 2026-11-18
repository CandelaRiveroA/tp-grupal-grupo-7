#ifndef SERVER_COMMANDS_CHAT_COMMAND_H
#define SERVER_COMMANDS_CHAT_COMMAND_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "common/common_server_status.h"
#include "common/common_command.h"
#include "domain/map/world.h"

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
    using Handler = std::string (ChatRouter::*)(World&, uint8_t,
                                               const std::vector<std::string>&);
    std::map<std::string, Handler> handlers_;

    std::string cmd_vida(World& world, uint8_t sender,
                         const std::vector<std::string>& args);
    std::string cmd_energia(World& world, uint8_t sender,
                            const std::vector<std::string>& args);
    std::string cmd_oro(World& world, uint8_t sender,
                        const std::vector<std::string>& args);

    static std::vector<std::string> split(const std::string& s);
    static std::string trim(const std::string& s);
    // Parseo de un argumento numérico único: false si no es un número válido
    // o si dejó basura después (p. ej. "12abc"). El float descarta nan/inf.
    static bool parse_int(const std::string& s, int& out);
    static bool parse_float(const std::string& s, float& out);
};

#endif  // SERVER_COMMANDS_CHAT_COMMAND_H