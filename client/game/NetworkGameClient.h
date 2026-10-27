#pragma once

#include <cstdint>
#include <map>

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
    NetworkGameClient(const char* host, const char* port);
    ~NetworkGameClient() override;

    void sendCommand(const protocol::ClientMessage& message) override;
    const LocalWorldView& worldState() const override;

private:
    struct RemotePlayer {
        int cellX = 0;
        int cellY = 0;
        world::Direction facing = world::Direction::Front;
        bool moving = false;
    };

    void rebuildInventory() const;

    Socket socket_;
    ProtocolSender sender_;
    ProtocolReceiver receiver_;
    Queue<protocol::ClientMessage> commands_;
    mutable Queue<ServerStatus> statuses_;
    ClientSender clientSender_;
    ClientReceiver clientReceiver_;

    uint8_t localPlayerId_;
    mutable std::map<uint8_t, RemotePlayer> players_;
    mutable LocalWorldView view_;
};

}  // namespace game
