#pragma once

#include <cstdint>
#include <optional>

#include "qt/services/SessionService.h"

// Crear y cargar tienen éxito. Unirse acepta un código válido, informa
// partida llena con "FULL22" y rechaza cualquier otro. Si se construye con un
// error, todos los pedidos fallan con ese error.
class FakeSessionService : public SessionService {
	Q_OBJECT

public:
	explicit FakeSessionService(
		std::optional<ServiceError> failure = std::nullopt,
		QObject* parent = nullptr);

	void createGame(const GameSelection& selection) override;
	void joinGame(const std::string& code, const std::string& username) override;
	void loadGame(std::uint32_t saveId) override;

private:
	static constexpr const char* kValidCode = "ABC234";
	static constexpr const char* kFullGameCode = "FULL22";
	static constexpr const char* kJoinedGameName = "Cooperativa";
	static constexpr std::uint32_t kJoinedWorldId = 1;

	std::optional<ServiceError> failure_;
};
