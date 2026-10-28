#pragma once

#include <QString>

#include <cstdint>
#include <map>
#include <string>

#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"

class QLineEdit;
class QLabel;
class QPushButton;
class SessionService;
class QWidget;

class JoinCodeScreen : public Screen {
	Q_OBJECT

public:
	explicit JoinCodeScreen(SessionService* sessions,
				QWidget* parent = nullptr);

	void onEnter() override;
	void onExit() override;

signals:
	void joinAccepted(std::string gameName);

private:
	SessionService* sessionService_;

	QLineEdit* code_;
	QPushButton* ok_;
	QPushButton* back_;
	QLabel* status_;
	std::map<ServiceError, QString> errorMessages_;

	bool joining_ = false;

	void buildMessages();
	void updateJoin();
	void onJoin();
	void onGameJoined(std::string gameName, std::uint32_t worldId);
	void onSessionFailed(ServiceError error);
	void showState(ScreenState state);
	QString messageFor(ServiceError error) const;
	bool canJoin() const;
};
