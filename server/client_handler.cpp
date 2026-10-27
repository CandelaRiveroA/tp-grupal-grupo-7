#include "client_handler.h"

#include <exception>
#include <utility>

#include <sys/socket.h>

ClientHandler::ClientHandler(Socket&& client_socket, uint8_t player_id, Queue<MatchEvent>& events):
        peer(std::move(client_socket)),
        player_id(player_id),
        events(events),
        statuses(std::make_shared<Queue<ServerStatus>>()),
        protocol_sender(peer),
        protocol_receiver(peer),
        receiver(protocol_receiver, events, player_id),
        sender(protocol_sender, *statuses) {}

void ClientHandler::run() {
    try {
        protocol_sender.send_welcome(ServerWelcome{player_id});
        events.push(MatchEvent(PlayerJoined{player_id, statuses}));
    } catch (const std::exception&) {
        return;
    }

    sender.start();
    receiver.start();
    receiver.join();

    statuses->close();
    sender.join();
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        peer.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
        // El cliente ya se desconecto por su lado.
    }
}
