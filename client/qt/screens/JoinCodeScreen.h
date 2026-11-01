#pragma once

#include <QBasicTimer>
#include <QString>

#include <cstdint>
#include <map>
#include <string>

#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"
#include "qt/theme/JoinCodeSprites.h"
#include "qt/theme/TitleBackdrop.h"

namespace core {
class MusicControl;
}

class QLineEdit;
class QLabel;
class QPaintEvent;
class QTimerEvent;
class SessionService;
class TitleButton;
class QWidget;

class JoinCodeScreen : public Screen {
	Q_OBJECT

public:
	explicit JoinCodeScreen(SessionService* sessions, QWidget* parent = nullptr,
				core::MusicControl* music = nullptr);

	void onEnter() override;
	void onExit() override;

signals:
	void joinAccepted(std::string gameName);

private:
	SessionService* sessionService_;

	core::MusicControl* music_;
	QLineEdit* code_;
	TitleButton* ok_;
	TitleButton* back_;
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

	void paintEvent(QPaintEvent* event) override;
	void timerEvent(QTimerEvent* event) override;

	TitleBackdrop backdrop_;
	QBasicTimer cloudTimer_;
	JoinCodeSprites joinSprites_;
};
