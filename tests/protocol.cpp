#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <thread>

#include "common/common_protocol.h"
#include "gtest/gtest.h"

namespace {

constexpr char TEST_HOST[] = "127.0.0.1";
constexpr char TEST_PORT[] = "45679";
constexpr char EXPECTED_MESSAGE[] = "hola mundo";
constexpr auto CONNECT_TIMEOUT = std::chrono::seconds(10);
constexpr auto CONNECT_RETRY_INTERVAL = std::chrono::milliseconds(20);

// El server corre en otro thread: si el connect se lanza antes de que ese
// thread termine de bind/listen, el SO responde ECONNREFUSED. Reintentamos
// hasta que el server este escuchando (o se agote el timeout).
std::unique_ptr<Protocol> connect_when_ready() {
    const auto deadline = std::chrono::steady_clock::now() + CONNECT_TIMEOUT;
    while (true) {
        try {
            return std::make_unique<Protocol>(TEST_HOST, TEST_PORT);
        } catch (const std::exception&) {
            if (std::chrono::steady_clock::now() >= deadline) {
                throw;
            }
            std::this_thread::sleep_for(CONNECT_RETRY_INTERVAL);
        }
    }
}

// cppcheck-suppress syntaxError
TEST(ProtocolTest, SendsAndReceivesFramedMessage) {
    std::exception_ptr server_error;

    std::thread server([&server_error]() {
        try {
            Protocol protocol(TEST_PORT);
            protocol.accept_connection();
            protocol.send_message(EXPECTED_MESSAGE);
        } catch (...) {
            server_error = std::current_exception();
        }
    });

    try {
        std::unique_ptr<Protocol> client = connect_when_ready();
        EXPECT_EQ(client->receive_message(), EXPECTED_MESSAGE);
    } catch (...) {
        server.join();
        throw;
    }
    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }
}

}  // namespace
