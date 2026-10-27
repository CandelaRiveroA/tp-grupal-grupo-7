#include "acceptor.h"

#include <exception>
#include <utility>

#include <sys/socket.h>

Acceptor::Acceptor(const char* service, Queue<MatchEvent>& events):
        listening(service), events(events), next_player_id(1) {}

void Acceptor::run() {
    while (should_keep_running()) {
        try {
            Socket peer = listening.accept();
            handlers.push_back(
                    std::make_unique<ClientHandler>(std::move(peer), next_player_id, events));
            ++next_player_id;
            handlers.back()->start();
        } catch (const std::exception&) {
            return;
        }
    }
}

void Acceptor::stop() {
    Thread::stop();
    try {
        listening.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
        // Si el socket ya estaba cerrado, accept() ya no esta bloqueado.
    }
}

void Acceptor::disconnect_clients() {
    for (auto& handler: handlers) {
        handler->stop();
    }
    for (auto& handler: handlers) {
        handler->join();
    }
}
