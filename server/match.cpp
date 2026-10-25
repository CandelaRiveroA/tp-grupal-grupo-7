#include "match.h"

#include <stdexcept>

Match::Match(Queue<Command>& commands, Queue<ServerStatus>& statuses):
        commands(commands), statuses(statuses) {}

void Match::execute() {
    while (should_keep_running()) {
        const Command command = commands.pop();
        if (command.type != CommandType::Move) {
            throw std::invalid_argument("El tipo de comando de la partida no es valido.");
        }
        statuses.push(ServerStatus(command.player_id, command.x, command.y, command.direction));
    }
}
