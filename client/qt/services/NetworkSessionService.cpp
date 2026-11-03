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
            {LobbyStatus::Full, ServiceError::GameFull},
            {LobbyStatus::UsernameTaken, ServiceError::UsernameTaken},
            {LobbyStatus::CharacterNameTaken, ServiceError::CharacterNameTaken}};

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
    sendRequest(MatchRequest{MatchRequestType::Create, selection.gameName, selection.gameName,
                             selection.username, selection.characterName});
}

void NetworkSessionService::joinGame(const std::string& code, const std::string& username,
                                     const std::string& characterName) {
    sendRequest(MatchRequest{MatchRequestType::Join, code, "", username, characterName});
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

void NetworkSessionService::sendRequest(const MatchRequest& request) {
    discardConnection();
    worker_ = std::thread([this, request]() { runRequest(request); });
}

void NetworkSessionService::runRequest(const MatchRequest& request) {
    try {
        Socket socket(core::kServerHost, core::kServerPort);
        socket.set_receive_timeout(kLobbyTimeoutSeconds);
        ProtocolSender sender(socket);
        ProtocolReceiver receiver(socket);
        sender.send_match_request(request);
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

    if (request.type == MatchRequestType::Create) {
        emit gameCreated();
    } else {
        emit gameJoined(request.codeMatch, kJoinedWorldId);
    }
}
