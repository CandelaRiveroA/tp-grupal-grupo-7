#pragma once

#include <string>

#include "qt/screens/Screen.h"

namespace core {
class MusicControl;
}

class PromptForm;

class UsernameScreen: public Screen {
	Q_OBJECT

public:
	explicit UsernameScreen(QWidget* parent = nullptr,
				core::MusicControl* music = nullptr);

	void onEnter() override;

signals:
	void usernameConfirmed(std::string username);

private:
	core::MusicControl* music_;
	PromptForm* form_;

	void updateConfirm();
	void onConfirm();
	bool canConfirm() const;
};
