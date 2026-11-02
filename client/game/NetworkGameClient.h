#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/common_command.h"
#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "common/common_queue.h"
#include "common/common_server_status.h"
#include "common/common_socket.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"

#include "client_receiver.h"
#include "client_sender.h"

namespace game {

class NetworkGameClient: public GameClient {
public:
    // toolUseDurationTicks: cuánto dura la animación de golpe en el cliente.
    // Es presentación: el servidor avisa cuándo se usó la herramienta.
    NetworkGameClient(Socket&& socket, int toolUseDurationTicks);
    ~NetworkGameClient() override;

    void sendCommand(const protocol::ClientMessage& message) override;
    void update(int iteration) override;
    const LocalWorldView& worldState() const override;
    bool connected() const override;

private:
    struct RemotePlayer {
        int cellX = 0;
        int cellY = 0;
        world::Direction facing = world::Direction::Front;
        bool moving = false;
        int toolUseRemaining = 0;
    };

    void apply(const ServerMessage& message);

    Socket socket_;
    ProtocolSender sender_;
    ProtocolReceiver receiver_;
    Queue<protocol::ClientMessage> commands_;
    Queue<ServerMessage> messages_;
    ClientSender clientSender_;
    ClientReceiver clientReceiver_;

    uint8_t localPlayerId_;
    int toolUseDurationTicks_;

    std::map<uint8_t, RemotePlayer> players_;
    std::map<std::pair<int, int>, ViewResource> resources_;
    std::map<std::pair<int, int>, ViewDroppedItem> dropped_;
    std::map<int, ViewInventorySlot> inventory_;
    // Eventos llegados desde el último update(); update() los pasa a view_.obtained.
    std::vector<ViewObtained> obtained_;
    LocalWorldView view_;
};

}  // namespace game
