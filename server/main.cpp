#include <exception>
#include <iostream>

#include "server.h"

#define SERVER_SERVICE_ARG_IDX 1
#define EXPECTED_SERVER_ARGS 2

int main(int argc, char* argv[]) {
    if (argc != EXPECTED_SERVER_ARGS) {
        std::cerr << "Uso: ./server <servicename>" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        Server server(argv[SERVER_SERVICE_ARG_IDX]);
        server.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error en el servidor: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Error desconocido en el servidor." << std::endl;
        return EXIT_FAILURE;
    }
}
