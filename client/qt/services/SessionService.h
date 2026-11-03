#pragma once

#include <QObject>

#include <cstdint>
#include <string>

#include "common/model/draft/GameSelection.h"
#include "qt/services/ServiceError.h"

// Crear, unirse a o cargar una partida. Cada pedido es asíncrono y termina
// en su señal de éxito o en sessionFailed().
class SessionService : public QObject {
	Q_OBJECT

public:
	explicit SessionService(QObject* parent = nullptr);
	~SessionService() override;

	virtual void createGame(const GameSelection& selection) = 0;
	virtual void joinGame(const std::string& code, const std::string& username) = 0;
	virtual void loadGame(std::uint32_t saveId) = 0;

signals:
	void gameCreated();
	// Al unirse, el mundo y el nombre de la partida ya vienen definidos.
	void gameJoined(std::string gameName, std::uint32_t worldId);
	void gameLoaded();
	void sessionFailed(ServiceError error);
};
