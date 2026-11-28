#include "client_handler.h"

#include <exception>
#include <string>
#include <utility>

#include <sys/socket.h>

ClientHandler::ClientHandler(Socket&& client_socket, MatchesMonitor& matches,
                             ServerOutputMonitor& output):
        peer(std::move(client_socket)),
        matches(matches),
        output(output),
        statuses(std::make_shared<Queue<ServerMessage>>()),
        protocol_sender(peer),
        protocol_receiver(peer),
        sender(protocol_sender, *statuses),
        peer_closed(false) {}

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
    serve();
    peer.close();
}

void ClientHandler::serve() {
    std::string codeMatch;
    MatchSeat seat{LobbyStatus::Invalid, 0, nullptr, {}};
    try {
        const MatchRequest request = protocol_receiver.receive_match_request();
        if (request.type == MatchRequestType::ListWorlds) {
            protocol_sender.send_world_list(matches.validWorldSummaries());
            return;
        }
        codeMatch = request.codeMatch;
        seat = request.type == MatchRequestType::Create ?
                       matches.create(codeMatch, request.worldName, request.username, request.characterName,
                                      statuses, request.appearance) :
                       matches.join(codeMatch, request.username, request.characterName, statuses,
                                    request.appearance);
        protocol_sender.send_lobby_status(seat.status);
        if (seat.status != LobbyStatus::Ok) {
            return;
        }
        protocol_sender.send_welcome(ServerWelcome{seat.player_id, seat.worldName});
    } catch (const std::exception& error) {
        if (seat.status == LobbyStatus::Ok) {
            matches.leave(codeMatch, seat.player_id);
        }
        if (should_keep_running()) {
            output.printError("Error iniciando cliente: ", error.what());
        }
        return;
    }

    ServerReceiver receiver(protocol_receiver, *seat.events, seat.player_id);
    sender.start();
    receiver.start();
    receiver.join();

    matches.leave(codeMatch, seat.player_id);

    sender.join();

    reportFailure(receiver);
    reportFailure(sender);
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        peer.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
        // El cliente ya se desconecto por su lado.
    }
}
