#include "JoinCodeScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QVBoxLayout>

#include <utility>

#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt/services/SessionService.h"
#include "qt/theme/JoinCodeRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/widgets/InputBox.h"
#include "qt/widgets/ParchmentBanner.h"
#include "qt/widgets/TitleButton.h"

JoinCodeScreen::JoinCodeScreen(SessionService* sessions, QWidget* parent,
			       core::MusicControl* music)
	: Screen(parent)
	, sessionService_(sessions)
	, music_(music)
	, code_(nullptr)
	, ok_(nullptr)
	, back_(nullptr)
	, status_(nullptr)
	, errorMessages_() {
	setObjectName("joinCode");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(96, 64, 28, 12);

	
	auto* central = new QWidget(this);
	auto* centralLayout = new QVBoxLayout(central);
	
	centralLayout->setSpacing(0);
	centralLayout->setContentsMargins(0, 0, 68, 0);
	layout->addWidget(central);

	centralLayout->addSpacing(JOIN_SCROLL_TOP_Y - 64);

	auto* prompt = new QLabel("Ingresá el código de invitación...");
	prompt->setObjectName("codePrompt");
	auto* banner = new ParchmentBanner(joinSprites_.scrollBanner(), prompt, central);
	banner->setObjectName("scrollBanner");
	centralLayout->addWidget(banner, 0, Qt::AlignHCenter);
	centralLayout->addSpacing(JOIN_SCROLL_ROW_GAP);

	auto* row = new QHBoxLayout();
	row->setSpacing(4);
	auto* box = new InputBox(
		joinSprites_.inputBox(),
		QMargins(JOIN_INPUT_PAD_LEFT, JOIN_INPUT_PAD_TOP,
			 JOIN_INPUT_PAD_RIGHT, JOIN_INPUT_PAD_BOTTOM),
		central);
	box->setObjectName("codeBox");
	code_ = box->edit();
	code_->setObjectName("codeInput");
	ok_ = new TitleButton(joinSprites_.okButton(), joinSprites_.okButtonHover(), "OK",
			      central);
	ok_->setObjectName("okButton");
	ok_->setEnabled(false);
	row->addStretch();
	row->addWidget(box);
	row->addWidget(ok_, 0, Qt::AlignVCenter);
	row->addStretch();
	centralLayout->addLayout(row);

	status_ = new QLabel(central);
	status_->setObjectName("screenStatus");
	status_->setAlignment(Qt::AlignCenter);
	status_->setWordWrap(true);
	status_->setVisible(false);
	centralLayout->addSpacing(JOIN_STATUS_ROW_GAP);
	centralLayout->addWidget(status_);

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
	connect(sessionService_, &SessionService::gameJoined, this,
		&JoinCodeScreen::onGameJoined);
	connect(sessionService_, &SessionService::sessionFailed, this,
		&JoinCodeScreen::onSessionFailed);

	buildMessages();
}

void JoinCodeScreen::onEnter() {
	code_->setFocus();
	Screen::onEnter();

	if (music_ != nullptr) {
		music_->play(core::MusicTrack::MenuTheme);
	}
}

void JoinCodeScreen::onExit() {
	joining_ = false;
	Screen::onExit();
}

void JoinCodeScreen::paintEvent(QPaintEvent* event) {
	Screen::paintEvent(event);
	QPainter painter(this);
	painter.fillRect(rect(), QColor(0, 0, 0, JOIN_BACKDROP_DIM_ALPHA));
}

void JoinCodeScreen::buildMessages() {
	errorMessages_[ServiceError::Network] =
		"No se pudo conectar con el servidor.";
	errorMessages_[ServiceError::InvalidCode] =
		"No hay ninguna partida con ese código.";
	errorMessages_[ServiceError::GameFull] = "La partida está llena.";
}

void JoinCodeScreen::updateJoin() { ok_->setEnabled(canJoin()); }

void JoinCodeScreen::onJoin() {
	if (!canJoin()) {
		return;
	}
	joining_ = true;
	showState(ScreenState::Loading);
	sessionService_->joinGame(code_->text().trimmed().toStdString());
}

void JoinCodeScreen::onGameJoined(std::string gameName, std::uint32_t) {
	if (!joining_) {
		return;
	}
	joining_ = false;
	showState(ScreenState::Ready);
	emit joinAccepted(std::move(gameName));
}

void JoinCodeScreen::onSessionFailed(ServiceError error) {
	if (!joining_) {
		// Falló otro pedido (crear, cargar): no es error de esta
		// pantalla.
		return;
	}
	joining_ = false;
	showState(ScreenState::Ready);

	QMessageBox message(QMessageBox::Warning,
			    "No se pudo unir a la partida", messageFor(error),
			    QMessageBox::Ok, this);
	message.exec();
}

void JoinCodeScreen::showState(ScreenState state) {
	const bool busy = state == ScreenState::Loading;
	code_->setEnabled(!busy);
	back_->setEnabled(!busy);
	ok_->setEnabled(!busy && canJoin());
	status_->setVisible(busy);
	if (busy) {
		status_->setText("Conectando...");
	}
}

QString JoinCodeScreen::messageFor(ServiceError error) const {
	const auto message = errorMessages_.find(error);
	if (message == errorMessages_.end()) {
		return "No se pudo unir a la partida.";
	}
	return message->second;
}

bool JoinCodeScreen::canJoin() const {
	return !joining_ && !code_->text().trimmed().isEmpty();
}
