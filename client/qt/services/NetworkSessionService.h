#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <thread>

#include "common/common_match_request.h"
#include "common/common_socket.h"
#include "qt/services/SessionService.h"

// Pedidos del lobby contra el servidor. Cada pedido usa su propia conexión:
// si el servidor la acepta, queda guardada para la partida (takeConnection).
class NetworkSessionService: public SessionService {
    Q_OBJECT

public:
    explicit NetworkSessionService(QObject* parent = nullptr);
    ~NetworkSessionService() override;

    void createGame(const GameSelection& selection) override;
    void joinGame(const std::string& code, const std::string& username,
                  const std::string& characterName, const CharacterAppearance& appearance) override;
    void loadGame(std::uint32_t saveId) override;

    // Socket ya unido a la partida. Solo válido después de gameCreated/gameJoined.
    std::optional<Socket> takeConnection();
    // Cierra la conexión guardada si el jugador vuelve sin entrar a jugar.
    void discardConnection();

private:
    std::thread worker_;
    std::optional<Socket> connection_;

    void sendRequest(const MatchRequest& request);
    void runRequest(const MatchRequest& request);
};
