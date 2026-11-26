#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "game/NetworkGameClient.h"
#include "world/ConfigError.h"

namespace {

// Servidor TCP local: entrega el welcome y un primer lote del protocolo real.
class StartupServer {
public:
    explicit StartupServer(const std::string& map, bool dayEnded = false,
                           bool newDay = false) {
        listener_ = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        socklen_t length = sizeof(address);
        if (listener_ < 0 || ::bind(listener_, reinterpret_cast<sockaddr*>(&address),
                                   sizeof(address)) != 0 ||
            ::listen(listener_, 1) != 0 ||
            ::getsockname(listener_, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
            if (listener_ >= 0) ::close(listener_);
            throw std::runtime_error("No se pudo abrir el servidor de prueba");
        }
        port_ = std::to_string(ntohs(address.sin_port));
        worker_ = std::thread([this, map, dayEnded, newDay]() {
            const int peer = ::accept(listener_, nullptr, nullptr);
            if (peer < 0) return;
            std::vector<uint8_t> bytes{static_cast<uint8_t>(ServerMessageType::Welcome),
                                       1, 0, 2, 'a', 'd'};
            if (dayEnded) bytes.push_back(static_cast<uint8_t>(ServerMessageType::DayEnd));
            if (newDay) bytes.push_back(static_cast<uint8_t>(ServerMessageType::Day));
            bytes.push_back(static_cast<uint8_t>(ServerMessageType::Reset));
            if (!map.empty()) {
                bytes.push_back(static_cast<uint8_t>(ServerMessageType::MapChange));
                bytes.push_back(static_cast<uint8_t>(map.size() >> 8));
                bytes.push_back(static_cast<uint8_t>(map.size() & 0xff));
                bytes.insert(bytes.end(), map.begin(), map.end());
            }
            bytes.push_back(static_cast<uint8_t>(ServerMessageType::SyncEnd));
            // Un byte por envío: el cliente no depende de los límites TCP.
            for (const uint8_t byte: bytes) {
                if (::send(peer, &byte, 1, MSG_NOSIGNAL) != 1) break;
            }
            char ignored;
            while (::recv(peer, &ignored, 1, 0) > 0) {}
            ::close(peer);
        });
    }

    ~StartupServer() {
        ::shutdown(listener_, SHUT_RDWR);
        ::close(listener_);
        worker_.join();
    }

    const char* port() const { return port_.c_str(); }

private:
    int listener_;
    std::string port_;
    std::thread worker_;
};

TEST(NetworkStartupTest, UsaLaCabanaAsignadaSinPerderElEventoDeFinDeDia) {
    StartupServer server("cabin_2", true);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", server.port()), 1, characters, "");
    EXPECT_EQ(client.initialMapId(), "cabin_2");
    EXPECT_EQ(client.worldName(), "ad");
    client.update(0);
    EXPECT_EQ(client.worldState().mapId, "cabin_2");
    EXPECT_TRUE(client.worldState().dayEnded);
    client.update(1);
    EXPECT_FALSE(client.worldState().dayEnded);
}

TEST(NetworkStartupTest, PrimerLoteSinMapaFallaAntesDeIniciarLosHilos) {
    StartupServer server("");
    const world::CharacterCatalog characters;
    EXPECT_THROW(game::NetworkGameClient(Socket("127.0.0.1", server.port()), 1, characters, ""),
                 world::ConfigError);
}

TEST(NetworkStartupTest, NuevoDiaSeEntregaConElLoteDeLaCabana) {
    StartupServer server("cabin_3", false, true);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", server.port()), 1, characters, "");
    client.update(0);
    EXPECT_TRUE(client.worldState().dayChanged);
    EXPECT_EQ(client.worldState().mapId, "cabin_3");
    client.update(1);
    EXPECT_FALSE(client.worldState().dayChanged);
}

}  // namespace
