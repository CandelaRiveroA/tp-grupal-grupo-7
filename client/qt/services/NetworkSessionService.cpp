#include "NetworkSessionService.h"

#include <QTimer>
#include <exception>
#include <unordered_map>
#include <utility>

#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "core/Constants.h"

namespace {

constexpr int kLobbyTimeoutSeconds = 5;

// shortcut: el servidor todavía tiene un solo mundo y no lo informa en el
// lobby; subir cuando haya mundos reales en el protocolo.
constexpr std::uint32_t kJoinedWorldId = 1;

ServiceError serviceErrorFor(LobbyStatus status) {
    static const std::unordered_map<LobbyStatus, ServiceError> statusToError = {
            {LobbyStatus::NameTaken, ServiceError::GameNameTaken},
            {LobbyStatus::Full, ServiceError::GameFull}};

    auto it = statusToError.find(status);
    if (it != statusToError.end()) {
        return it->second;
    }

    return ServiceError::InvalidCode;
}

}  // namespace

NetworkSessionService::NetworkSessionService(QObject* parent): SessionService(parent) {}

NetworkSessionService::~NetworkSessionService() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

void NetworkSessionService::createGame(const GameSelection& selection) {
    sendRequest(MatchRequestType::Create, selection.gameName);
}

void NetworkSessionService::joinGame(const std::string& code) {
    sendRequest(MatchRequestType::Join, code);
}

void NetworkSessionService::loadGame(std::uint32_t) {
    // shortcut: el servidor no guarda partidas todavía, así que cargar siempre
    // falla; subir cuando el protocolo tenga guardados.
    QTimer::singleShot(0, this, [this]() { emit sessionFailed(ServiceError::SaveNotFound); });
}

std::optional<Socket> NetworkSessionService::takeConnection() {
    std::optional<Socket> connection = std::move(connection_);
    connection_.reset();
    return connection;
}

void NetworkSessionService::discardConnection() {
    if (worker_.joinable()) {
        worker_.join();
    }
    connection_.reset();
}

void NetworkSessionService::sendRequest(MatchRequestType type, const std::string& name) {
    discardConnection();
    worker_ = std::thread([this, type, name]() { runRequest(type, name); });
}

void NetworkSessionService::runRequest(MatchRequestType type, const std::string& name) {
    try {
        Socket socket(core::kServerHost, core::kServerPort);
        socket.set_receive_timeout(kLobbyTimeoutSeconds);
        ProtocolSender sender(socket);
        ProtocolReceiver receiver(socket);
        sender.send_match_request(MatchRequest{type, name});
        const LobbyStatus status = receiver.receive_lobby_status();
        if (status != LobbyStatus::Ok) {
            emit sessionFailed(serviceErrorFor(status));
            return;
        }
        socket.set_receive_timeout(0);
        connection_.emplace(std::move(socket));
    } catch (const std::exception&) {
        emit sessionFailed(ServiceError::Network);
        return;
    }

    if (type == MatchRequestType::Create) {
        emit gameCreated();
    } else {
        emit gameJoined(name, kJoinedWorldId);
    }
}
