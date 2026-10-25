#include <chrono>
#include <exception>
#include <optional>
#include <stdexcept>
#include <thread>

#include "common/common_command.h"
#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "common/common_thread.h"
#include "gtest/gtest.h"

class ProtocolTest: public testing::Test {
public:
    class ServerThread: public Thread {
    private:
        const char* port;
        PlayerId expected_player_id;
        Command& first_command;
        Command& second_command;
        std::exception_ptr& error;

    public:
        ServerThread(const char* port, PlayerId expected_player_id, Command& first_command,
                     Command& second_command, std::exception_ptr& error):
                port(port),
                expected_player_id(expected_player_id),
                first_command(first_command),
                second_command(second_command),
                error(error) {}

        void run() override {
            try {
                Socket listening_socket(port);
                Socket connection = listening_socket.accept();
                ProtocolReceiver receiver(connection, expected_player_id);

                const std::optional<Command> first = receiver.receive_command();
                if (!first.has_value()) {
                    throw std::runtime_error("Falto el primer comando.");
                }
                first_command = first.value();

                const std::optional<Command> second = receiver.receive_command();
                if (!second.has_value()) {
                    throw std::runtime_error("Falto el segundo comando.");
                }
                second_command = second.value();
            } catch (...) {
                error = std::current_exception();
            }
        }
    };

    Socket connect_when_ready(const char* host, const char* port) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (true) {
            try {
                return Socket(host, port);
            } catch (const std::exception&) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    throw;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    }

protected:
    const char* test_host() const { return "127.0.0.1"; }
    const char* test_port() const { return "45679"; }
};

TEST_F(ProtocolTest, SendsAndReceivesConsecutiveMoveCommands) {
    Command first_command(1, CommandType::Move, 0, 0, Direction::Up);
    Command second_command(1, CommandType::Move, 0, 0, Direction::Up);
    std::exception_ptr server_error;
    ServerThread server(test_port(), 1, first_command, second_command, server_error);
    server.start();

    Socket connection = connect_when_ready(test_host(), test_port());
    ProtocolSender sender(connection);
    sender.send_move_command(1, 12, 34, Direction::Left);
    sender.send_move_command(1, 200, 250, Direction::Right);

    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }

    EXPECT_EQ(first_command.player_id, 1);
    EXPECT_EQ(first_command.type, CommandType::Move);
    EXPECT_EQ(first_command.x, 12);
    EXPECT_EQ(first_command.y, 34);
    EXPECT_EQ(first_command.direction, Direction::Left);

    EXPECT_EQ(second_command.player_id, 1);
    EXPECT_EQ(second_command.type, CommandType::Move);
    EXPECT_EQ(second_command.x, 200);
    EXPECT_EQ(second_command.y, 250);
    EXPECT_EQ(second_command.direction, Direction::Right);
}

TEST_F(ProtocolTest, RejectsACommandFromAnotherPlayer) {
    Command first_command(1, CommandType::Move, 0, 0, Direction::Up);
    Command second_command(1, CommandType::Move, 0, 0, Direction::Up);
    std::exception_ptr server_error;
    ServerThread server(test_port(), 1, first_command, second_command, server_error);
    server.start();

    Socket connection = connect_when_ready(test_host(), test_port());
    const uint8_t invalid_command[] = {1, 2, 0, 0, 0};
    ASSERT_EQ(connection.sendall(invalid_command, sizeof(invalid_command)),
              static_cast<int>(sizeof(invalid_command)));

    server.join();

    ASSERT_TRUE(server_error);
    EXPECT_THROW(std::rethrow_exception(server_error), std::invalid_argument);
}

TEST_F(ProtocolTest, RejectsAnInvalidDirection) {
    Command first_command(1, CommandType::Move, 0, 0, Direction::Up);
    Command second_command(1, CommandType::Move, 0, 0, Direction::Up);
    std::exception_ptr server_error;
    ServerThread server(test_port(), 1, first_command, second_command, server_error);
    server.start();

    Socket connection = connect_when_ready(test_host(), test_port());
    const uint8_t invalid_command[] = {1, 1, 0, 0, 4};
    ASSERT_EQ(connection.sendall(invalid_command, sizeof(invalid_command)),
              static_cast<int>(sizeof(invalid_command)));

    server.join();

    ASSERT_TRUE(server_error);
    EXPECT_THROW(std::rethrow_exception(server_error), std::invalid_argument);
}

TEST_F(ProtocolTest, RejectsATruncatedCommand) {
    Command first_command(1, CommandType::Move, 0, 0, Direction::Up);
    Command second_command(1, CommandType::Move, 0, 0, Direction::Up);
    std::exception_ptr server_error;
    ServerThread server(test_port(), 1, first_command, second_command, server_error);
    server.start();

    Socket connection = connect_when_ready(test_host(), test_port());
    const uint8_t truncated_command[] = {1, 1, 10, 20};
    ASSERT_EQ(connection.sendall(truncated_command, sizeof(truncated_command)),
              static_cast<int>(sizeof(truncated_command)));
    connection.shutdown(1);

    server.join();

    ASSERT_TRUE(server_error);
    EXPECT_THROW(std::rethrow_exception(server_error), std::exception);
}
