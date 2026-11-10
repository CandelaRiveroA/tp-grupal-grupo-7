#include "acceptor.h"

#include <exception>
#include <utility>

#include <sys/socket.h>

#include "../common/common_liberror.h"

Acceptor::Acceptor(const char* service, MatchesMonitor& matches, ServerOutputMonitor& output):
        listening(service), matches(matches), output(output) {}

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
    while (should_keep_running()) {
        try {
            Socket peer = listening.accept();
            collectFinishedHandlers();
            handlers.push_back(std::make_unique<ClientHandler>(std::move(peer), matches, output));
            handlers.back()->start();
        } catch (const LibError& error) {
            if (should_keep_running()) {
                output.printError("Error aceptando conexion: ", error.what());
            }
            break;
        } catch (const std::exception& error) {
            if (should_keep_running()) {
                output.printError("Error inesperado en acceptor: ", error.what());
            }
            break;
        } catch (...) {
            if (should_keep_running()) {
                output.printError("Error inesperado en acceptor: ", "<Desconocido>");
            }
            break;
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
