#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "game/NetworkGameClient.h"
#include "game/CarrySelection.h"
#include "protocol/mock_socket.h"

namespace {

class BroadcastPeer {
private:
    int listener_;
    std::string port_;
    std::thread worker_;
    std::atomic<bool> matched_{false};
    std::atomic<bool> afterMatched_{false};

public:
    BroadcastPeer(std::vector<uint8_t> outgoing, std::vector<uint8_t> expected = {},
                  std::vector<uint8_t> expectedAfter = {}) {
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
            throw std::runtime_error("No se pudo iniciar la conexión de prueba");
        }
        port_ = std::to_string(ntohs(address.sin_port));
        worker_ = std::thread([this, outgoing = std::move(outgoing),
                               expected = std::move(expected), expectedAfter = std::move(expectedAfter)]() {
            const int peer = ::accept(listener_, nullptr, nullptr);
            if (peer < 0) return;
            const timeval timeout{5, 0};
            ::setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            bool matched = true;
            for (uint8_t expectedByte: expected) {
                uint8_t byte = 0;
                if (::recv(peer, &byte, 1, MSG_WAITALL) != 1 || byte != expectedByte) {
                    matched = false;
                    break;
                }
            }
            matched_ = matched;
            if (matched) {
                for (uint8_t byte: outgoing) {
                    if (::send(peer, &byte, 1, MSG_NOSIGNAL) != 1) break;
                }
                for (uint8_t expectedByte: expectedAfter) {
                    uint8_t byte = 0;
                    if (::recv(peer, &byte, 1, MSG_WAITALL) != 1 || byte != expectedByte) {
                        matched = false;
                        break;
                    }
                }
                afterMatched_ = matched;
                char ignored;
                while (::recv(peer, &ignored, 1, 0) > 0) {}
            }
            ::close(peer);
        });
    }

    ~BroadcastPeer() {
        ::shutdown(listener_, SHUT_RDWR);
        ::close(listener_);
        worker_.join();
    }

    const char* port() const { return port_.c_str(); }
    bool matched() const { return matched_; }
    bool afterMatched() const { return afterMatched_; }
};

class BroadcastPackets {
public:
    std::vector<uint8_t> bytes;

    void number(uint16_t value) {
        bytes.push_back(static_cast<uint8_t>(value >> 8));
        bytes.push_back(static_cast<uint8_t>(value & 0xff));
    }
    void string(const std::string& value) {
        number(static_cast<uint16_t>(value.size()));
        bytes.insert(bytes.end(), value.begin(), value.end());
    }
    void held(uint8_t id, const std::string& item) {
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::PlayerHeld));
        bytes.push_back(id);
        string(item);
    }
    void status(uint8_t id, uint16_t x) {
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::Status));
        bytes.push_back(id);
        number(x);
        number(1);
        bytes.push_back(static_cast<uint8_t>(Direction::DOWN));
        bytes.insert(bytes.end(), 8, 0);  // apariencia
        number(100);
        number(100);
        number(10000);
        number(10000);
        bytes.push_back(0);
    }
    void snapshot(const std::string& item) {
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::Welcome));
        bytes.push_back(1);
        string("ad");
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::GiftAccepted));
        bytes.push_back(2);
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::Reset));
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::MapChange));
        string("cabin_1");
        held(1, "");
        status(1, 1);
        held(2, item);
        status(2, 2);
        bytes.push_back(static_cast<uint8_t>(ServerMessageType::SyncEnd));
    }
};

TEST(BroadcastNetworkTest, ProtocoloDeSeleccionManosYCorazonMantieneLosLimitesDeMensajes) {
    BroadcastPackets packets;
    packets.held(2, "strawberry_seeds");
    packets.held(3, "");
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::GiftAccepted));
    packets.bytes.push_back(2);
    BroadcastPeer peer(packets.bytes,
                       {static_cast<uint8_t>(protocol::ClientMessageType::SelectSlot), 4});
    Socket socket("127.0.0.1", peer.port());
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    sender.send_client_message(protocol::SelectSlotMessage{4});
    auto message = receiver.receive_server_message();
    ASSERT_TRUE(std::holds_alternative<ServerPlayerHeld>(message));
    EXPECT_EQ(std::get<ServerPlayerHeld>(message).content_id, "strawberry_seeds");
    message = receiver.receive_server_message();
    EXPECT_TRUE(std::get<ServerPlayerHeld>(message).content_id.empty());
    message = receiver.receive_server_message();
    EXPECT_EQ(std::get<ServerGiftAccepted>(message).receiver_id, 2);
    EXPECT_TRUE(peer.matched());
}

TEST(BroadcastNetworkTest, ClienteAplicaElObjetoRemotoYElEventoDeRegaloUnaSolaVez) {
    BroadcastPackets packets;
    packets.snapshot("strawberry_seeds");
    BroadcastPeer peer(packets.bytes);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", peer.port()), 1, characters, "");
    client.update(0);
    const auto& view = client.worldState();
    ASSERT_EQ(view.entities.size(), 2u);
    EXPECT_EQ(view.entities[0].id, 1);
    EXPECT_FALSE(view.entities[0].carriedContent.has_value());
    EXPECT_EQ(view.entities[1].id, 2);
    EXPECT_EQ(view.entities[1].carriedContent,
              std::optional<std::string>{"strawberry_seeds"});
    ASSERT_EQ(view.giftAcceptedPlayers.size(), 1u);
    EXPECT_EQ(view.giftAcceptedPlayers[0], 2);
    client.update(1);
    EXPECT_TRUE(client.worldState().giftAcceptedPlayers.empty());
    EXPECT_EQ(client.worldState().entities[1].carriedContent,
              std::optional<std::string>{"strawberry_seeds"});
}

TEST(BroadcastNetworkTest, ServidorSerializaLosAvisosYRecibeLaSeleccion) {
    BroadcastPeer peer(
            {static_cast<uint8_t>(protocol::ClientMessageType::SelectSlot), 7},
            {static_cast<uint8_t>(ServerMessageType::PlayerHeld), 2, 0, 3, 'e', 'g', 'g',
             static_cast<uint8_t>(ServerMessageType::GiftAccepted), 2});
    Socket socket("127.0.0.1", peer.port());
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    sender.send_server_message(ServerPlayerHeld{2, "egg"});
    sender.send_server_message(ServerGiftAccepted{2});
    const auto command = receiver.receive_command();
    ASSERT_TRUE(command.has_value());
    ASSERT_TRUE(std::holds_alternative<protocol::SelectSlotMessage>(*command));
    EXPECT_EQ(std::get<protocol::SelectSlotMessage>(*command).slot, 7);
    EXPECT_TRUE(peer.matched());
}

TEST(BroadcastNetworkTest, ContenidoVacioEnElSnapshotDejaLasManosRemotasLibres) {
    BroadcastPackets packets;
    packets.snapshot("");
    BroadcastPeer peer(packets.bytes);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", peer.port()), 1, characters, "");
    client.update(0);
    ASSERT_EQ(client.worldState().entities.size(), 2u);
    EXPECT_FALSE(client.worldState().entities[1].carriedContent.has_value());
}

TEST(BroadcastNetworkTest, NombresPublicosYAvisoRealLleganALaVista) {
    BroadcastPackets packets;
    // Eventos antes del primer lote: también cubre jugadores que entran tarde.
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::Welcome));
    packets.bytes.push_back(1);
    packets.string("ad");
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::PlayerName));
    packets.bytes.push_back(2);
    packets.string("Miguel");
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::InventoryFull));
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::DayEnd));
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::Reset));
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::MapChange));
    packets.string("cabin_1");
    packets.status(1, 1);
    packets.status(2, 2);
    packets.bytes.back() = 1;  // desmayo por horario con vida intacta
    packets.bytes.push_back(static_cast<uint8_t>(ServerMessageType::SyncEnd));
    BroadcastPeer peer(packets.bytes);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", peer.port()), 1, characters, "Ana");
    client.update(0);
    const auto& view = client.worldState();
    ASSERT_EQ(view.entities.size(), 2u);
    EXPECT_EQ(view.entities[1].displayName, "Miguel");
    EXPECT_EQ(view.entities[1].action, world::ActionKind::Faint);
    ASSERT_EQ(view.notices.size(), 1u);
    EXPECT_EQ(view.notices[0].id, "inventory_full");
    EXPECT_TRUE(view.dayEnded);
    client.update(1);
    EXPECT_TRUE(client.worldState().notices.empty());
    EXPECT_EQ(client.worldState().entities[1].displayName, "Miguel");
}


std::vector<uint8_t> containerSnapshotBytes() {
    MockSocket buffer;
    ProtocolSender sender(buffer);
    sender.send_welcome(ServerWelcome{1, "ad"});
    sender.send_server_message(ServerReset{});
    sender.send_server_message(ServerMapChange{"farmhouse"});
    sender.send_server_message(ServerStatus{1, 1, 1, Direction::DOWN, {}, 100, 100, 10000, 10000, 0});
    sender.send_server_message(ServerFurniture{2, 1, "cofre", 2, false, true});
    sender.send_server_message(ServerContainerState{true, 2, 1, {{0, 3, "milk", true}}});
    sender.send_server_message(ServerInventorySlot{8, 1, "cofre", true, 1, 1});
    sender.send_server_message(ServerWorldPlayers{{{1, "farmhouse", 1, 1, {}}, {2, "farm", 8, 9, {}}}});
    sender.send_server_message(ServerSyncEnd{});
    return buffer.sent_bytes();
}

TEST(BroadcastNetworkTest, ClienteDeRedLlenaElCofrePrivadoYElMapaGlobal) {
    BroadcastPeer peer(containerSnapshotBytes());
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", peer.port()), 1, characters, "Ana");
    client.update(0);
    const auto& state = client.worldState();
    ASSERT_TRUE(state.openContainer.has_value());
    EXPECT_EQ(state.openContainer->column, 2);
    ASSERT_EQ(state.containerSlots.size(), 1u);
    EXPECT_EQ(state.containerSlots[0].contentId, "milk");
    EXPECT_EQ(state.containerSlots[0].count, 3);
    ASSERT_EQ(state.worldMapPlayers.size(), 2u);
    EXPECT_TRUE(state.worldMapPlayers[0].isLocal);
    EXPECT_FALSE(state.worldMapPlayers[1].isLocal);
    EXPECT_EQ(state.worldMapPlayers[1].mapId, "farm");
    ASSERT_EQ(state.placedFurniture.size(), 1u);
    EXPECT_TRUE(state.placedFurniture[0].open);
    EXPECT_EQ(state.placedFurniture[0].facing, world::FurnitureFacing::Back);
    const world::ContentTraits traits;
    game::CarrySelection carry(traits);
    carry.observe(state, 8);
    ASSERT_TRUE(carry.heldFootprint().has_value());
    EXPECT_EQ(carry.heldFootprint()->columns, 1);
    EXPECT_EQ(carry.held(), std::optional<std::string>{"cofre"});
}

TEST(BroadcastNetworkTest, IntencionesSDLSeEncolanComoMensajesDelProtocolo) {
    const std::vector<uint8_t> expected{
        10, 4, 1,                       // tacho
        14, 0, 3, 0, 4, 8, 2,         // colocar cofre
        15, 0, 2, 0, 1,               // recoger cofre
        13, 0, 2, 0, 1, 1, 4, 1,     // depositar una unidad
        11, 0, 2, 0, 1,               // abrir
        12                            // cerrar
    };
    BroadcastPeer peer(containerSnapshotBytes(), {}, expected);
    const world::CharacterCatalog characters;
    game::NetworkGameClient client(Socket("127.0.0.1", peer.port()), 1, characters, "Ana");
    client.update(0);
    client.requestDiscard({4, 1});
    client.requestPlace({{3, 4}, 8, world::FurnitureFacing::Back});
    client.requestGrab({"cofre", {2, 1}});
    client.requestTransfer({"cofre@2,1", game::TransferDirection::ToContainer, 4, 1});
    client.sendCommand(protocol::OpenContainerMessage{2, 1});
    client.sendCommand(protocol::CloseContainerMessage{});
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (!peer.afterMatched() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::yield();
    }
    EXPECT_TRUE(peer.afterMatched());
}

}  // namespace
