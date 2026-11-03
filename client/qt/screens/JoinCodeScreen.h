#pragma once

#include <string>

#include "qt/screens/Screen.h"

namespace core {
class MusicControl;
}

class QLineEdit;
class QPaintEvent;
class TitleButton;
class QWidget;

class JoinCodeScreen : public Screen {
	Q_OBJECT

public:
	explicit JoinCodeScreen(QWidget* parent = nullptr,
				core::MusicControl* music = nullptr);

	void onEnter() override;

signals:
	void joinAccepted(std::string code);

private:
	core::MusicControl* music_;
	QLineEdit* code_;
	TitleButton* ok_;
	TitleButton* back_;

	void updateJoin();
	void onJoin();
	bool canJoin() const;

	void paintEvent(QPaintEvent* event) override;
};
