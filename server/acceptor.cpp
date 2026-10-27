#include "acceptor.h"

#include <exception>
#include <utility>

#include <sys/socket.h>

Acceptor::Acceptor(const char* service, Queue<MatchEvent>& events, ServerOutputMonitor& output):
        listening(service), events(events), output(output), next_player_id(1) {}

void Acceptor::collectFinishedHandlers() {
    auto it = handlers.begin();
    while (it != handlers.end()) {
        if (!(*it)->is_alive()) {
            (*it)->join();
            it = handlers.erase(it);
        } else {
            ++it;
        }
    }
}

void Acceptor::clear() {
    for (auto& handler: handlers) {
        handler->stop();
        handler->join();
    }
    handlers.clear();
}

void Acceptor::run() {
    try {
        while (should_keep_running()) {
            Socket peer = listening.accept();
            collectFinishedHandlers();
            if (handlers.size() >= static_cast<size_t>(MAX_JUGADORES)) {
                output.printError("Conexion rechazada: ", "la partida esta llena.");
                continue;
            }
            handlers.push_back(std::make_unique<ClientHandler>(std::move(peer), next_player_id,
                                                               events, output));
            handlers.back()->start();
            ++next_player_id;
        }
    } catch (const std::exception& error) {
        if (should_keep_running()) {
            output.printError("Error aceptando conexion: ", error.what());
        }
    }

    clear();
}

void Acceptor::stop() {
    Thread::stop();
    try {
        listening.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
        // Si el socket ya estaba cerrado, accept() ya no esta bloqueado.
    }
}
