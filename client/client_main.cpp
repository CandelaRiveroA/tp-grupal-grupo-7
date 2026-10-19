#include <exception>
#include <iostream>

#include "client.h"

#define CLIENT_HOSTNAME_ARG_IDX 1
#define CLIENT_SERVICE_ARG_IDX 2
#define EXPECTED_CLIENT_ARGS 3

int main(int argc, char* argv[]) {
    if (argc != EXPECTED_CLIENT_ARGS) {
        std::cerr << "Uso: ./client <hostname> <servicename>" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        const char* hostname = argv[CLIENT_HOSTNAME_ARG_IDX];
        const char* service = argv[CLIENT_SERVICE_ARG_IDX];

        Client client(hostname, service);
        client.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error en el cliente: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Error desconocido en el cliente." << std::endl;
        return EXIT_FAILURE;
    }
}
