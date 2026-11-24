#include "UsernameScreen.h"

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt_theme/TitleSpriteRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/theme/UsernameRects.h"
#include "qt/widgets/PromptForm.h"
#include "qt_theme/TitleButton.h"

UsernameScreen::UsernameScreen(QWidget* parent, core::MusicControl* music)
	: Screen(parent)
	, music_(music)
	, form_(nullptr) {
	setObjectName("username");

	TitleSprites sprites;
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, TITLE_LOGO_TOP_Y, 0, 0);
	layout->setSpacing(0);

	auto* logo = new QLabel(this);
	logo->setObjectName("gameLogo");
	logo->setPixmap(sprites.logo());
	logo->setFixedSize(TITLE_LOGO_W, TITLE_LOGO_H);
	logo->setAlignment(Qt::AlignCenter);
	layout->addWidget(logo, 0, Qt::AlignHCenter);

	layout->addSpacing(USERNAME_LOGO_FORM_GAP);

	form_ = new PromptForm("Ingresá tu usuario", this);
	form_->setObjectName("usernameForm");
	form_->prompt()->setObjectName("usernamePrompt");
	form_->edit()->setObjectName("usernameInput");
	form_->okButton()->setObjectName("okButton");
	form_->okButton()->setEnabled(false);
	layout->addWidget(form_);

	layout->addStretch();

	connect(form_->edit(), &QLineEdit::textChanged, this,
		&UsernameScreen::updateConfirm);
	connect(form_->edit(), &QLineEdit::returnPressed, this,
		&UsernameScreen::onConfirm);
	connect(form_->okButton(), &QAbstractButton::clicked, this,
		&UsernameScreen::onConfirm);
}

void UsernameScreen::onEnter() {
	form_->edit()->setFocus();
	Screen::onEnter();

	if (music_ != nullptr) {
		music_->play(core::MusicTrack::MenuTheme);
	}
}

void UsernameScreen::updateConfirm() {
	form_->okButton()->setEnabled(canConfirm());
}

bool UsernameScreen::canConfirm() const {
	return !form_->edit()->text().trimmed().isEmpty();
}

void UsernameScreen::onConfirm() {
	if (!canConfirm()) {
		return;
	}
	emit usernameConfirmed(form_->edit()->text().trimmed().toStdString());
}
