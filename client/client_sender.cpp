#include "client_sender.h"

#include <stdexcept>

ClientSender::ClientSender(ProtocolSender& protocol, Queue<Command>& commands):
        protocol(protocol),
        commands(commands),
        handlers{{CommandType::Move, &ClientSender::handle_move_command}} {}

void ClientSender::handle_move_command(const Command& command) {
    protocol.send_move_command(command.player_id, command.x, command.y, command.direction);
}

void ClientSender::execute() {
    while (should_keep_running()) {
        const Command command = commands.pop();

        const auto handler = handlers.find(command.type);
        if (handler == handlers.end()) {
            throw std::invalid_argument("El tipo de comando local no es valido.");
        }
        (this->*(handler->second))(command);
    }
}
