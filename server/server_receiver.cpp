#include "server_receiver.h"

#include <stdexcept>

ServerReceiver::ServerReceiver(ProtocolReceiver& protocol, Queue<Command>& commands):
        protocol(protocol),
        commands(commands),
        handlers{{CommandType::Move, &ServerReceiver::handle_move_command}} {}

void ServerReceiver::handle_move_command(const Command& command) { commands.push(command); }

void ServerReceiver::execute() {
    while (should_keep_running()) {
        std::optional<Command> command = protocol.receive_command();
        if (!command.has_value()) {
            return;
        }

        const auto handler = handlers.find(command->type);
        if (handler == handlers.end()) {
            throw std::invalid_argument("El tipo de comando recibido no es valido.");
        }
        (this->*(handler->second))(command.value());
    }
}
