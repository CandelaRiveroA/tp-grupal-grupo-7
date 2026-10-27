#include "client_handler.h"

#include <exception>
#include <utility>

#include <sys/socket.h>

ClientHandler::ClientHandler(Socket&& client_socket, uint8_t player_id, Queue<MatchEvent>& events,
                             ServerOutputMonitor& output):
        peer(std::move(client_socket)),
        player_id(player_id),
        events(events),
        output(output),
        statuses(std::make_shared<Queue<ServerMessage>>()),
        protocol_sender(peer),
        protocol_receiver(peer),
        receiver(protocol_receiver, events, player_id),
        sender(protocol_sender, *statuses) {}

void ClientHandler::reportFailure(const Worker& worker) {
    if (!worker.failed() || !should_keep_running()) {
        return;
    }
    try {
        std::rethrow_exception(worker.exception());
    } catch (const std::exception& error) {
        output.printError("Error de comunicacion con cliente: ", error.what());
    }
}

void ClientHandler::run() {
    try {
        protocol_sender.send_welcome(ServerWelcome{player_id});
        events.push(MatchEvent(PlayerJoined{player_id, statuses}));
    } catch (const std::exception& error) {
        if (should_keep_running()) {
            output.printError("Error iniciando cliente: ", error.what());
        }
        return;
    }

    sender.start();
    receiver.start();
    receiver.join();

    try {
        events.push(MatchEvent(PlayerLeft{player_id}));
    } catch (const ClosedQueue&) {
        statuses->close();
    }

    sender.join();

    reportFailure(receiver);
    reportFailure(sender);
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        peer.shutdown(SHUT_RD);
    } catch (const std::exception&) {
        // El cliente ya se desconecto por su lado.
    }
}
