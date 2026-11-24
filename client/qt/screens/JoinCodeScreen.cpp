#include "JoinCodeScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>

#include <utility>

#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt/theme/JoinCodeRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/widgets/PromptForm.h"
#include "qt_theme/TitleButton.h"

JoinCodeScreen::JoinCodeScreen(QWidget* parent, core::MusicControl* music)
	: Screen(parent)
	, music_(music)
	, code_(nullptr)
	, ok_(nullptr)
	, back_(nullptr) {
	setObjectName("joinCode");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(JOIN_MARGIN_LEFT, JOIN_MARGIN_TOP, JOIN_MARGIN_RIGHT,
				  JOIN_MARGIN_BOTTOM);

	
	auto* central = new QWidget(this);
	auto* centralLayout = new QVBoxLayout(central);
	
	centralLayout->setSpacing(0);
	centralLayout->setContentsMargins(0, 0, JOIN_CENTRAL_RIGHT_MARGIN, 0);
	layout->addWidget(central);

	centralLayout->addSpacing(JOIN_SCROLL_TOP_Y - JOIN_MARGIN_TOP);

	auto* form = new PromptForm("Ingresá el código de invitación...", central);
	form->prompt()->setObjectName("codePrompt");
	form->banner()->setObjectName("scrollBanner");
	form->inputBox()->setObjectName("codeBox");
	code_ = form->edit();
	code_->setObjectName("codeInput");
	ok_ = form->okButton();
	ok_->setObjectName("okButton");
	ok_->setEnabled(false);
	centralLayout->addWidget(form);

	layout->addStretch();

	auto* buttons = new QHBoxLayout();
	TitleSprites sprites;
	back_ = new TitleButton(sprites.backButton(), sprites.backButtonHover(), "Volver", this);
	back_->setObjectName("backButton");
	connect(back_, &QAbstractButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::MainMenu);
	});
	buttons->addStretch();
	buttons->addWidget(back_);
	layout->addLayout(buttons);

	connect(code_, &QLineEdit::textChanged, this,
		&JoinCodeScreen::updateJoin);
	connect(code_, &QLineEdit::returnPressed, this, &JoinCodeScreen::onJoin);
	connect(ok_, &QAbstractButton::clicked, this, &JoinCodeScreen::onJoin);
}

void JoinCodeScreen::onEnter() {
	code_->setFocus();
	Screen::onEnter();

	if (music_ != nullptr) {
		music_->play(core::MusicTrack::MenuTheme);
	}
}

void JoinCodeScreen::paintEvent(QPaintEvent* event) {
	Screen::paintEvent(event);
	QPainter painter(this);
	painter.fillRect(rect(), QColor(0, 0, 0, JOIN_BACKDROP_DIM_ALPHA));
}

void JoinCodeScreen::updateJoin() { ok_->setEnabled(canJoin()); }

void JoinCodeScreen::onJoin() {
	if (!canJoin()) {
		return;
	}
	emit joinAccepted(code_->text().trimmed().toStdString());
}

bool JoinCodeScreen::canJoin() const { return !code_->text().trimmed().isEmpty(); }
