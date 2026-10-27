// TODO(tp2): punto de entrada provisional del servidor.
// Reemplazar por la construccion de Server / acceptor y el loop de aceptacion
// de clientes.
#include <exception>

#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    try {
        Server server(argv[1]);
        server.run();
    } catch (const std::exception&) {
        return 1;
    }

    return 0;
}
