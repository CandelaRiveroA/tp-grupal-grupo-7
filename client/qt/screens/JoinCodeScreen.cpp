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
#include "qt/widgets/PromptForm.h"
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
	code_->setMaxLength(InviteCode::kLength);
	ok_ = form->okButton();
	ok_->setObjectName("okButton");
	ok_->setEnabled(false);
	centralLayout->addWidget(form);

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
		&JoinCodeScreen::onCodeChanged);
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

void JoinCodeScreen::onCodeChanged() {
	// Los códigos del formato son mayúsculas: lo tipeado en minúsculas se
	// normaliza al vuelo, así el tipeo natural pasa la validación.
	const QString normalized = code_->text().toUpper();
	if (code_->text() == normalized) {
		updateJoin();
		return;
	}
	const int cursor = code_->cursorPosition();
	code_->setText(normalized);
	code_->setCursorPosition(cursor);
	// setText re-dispara textChanged: la decisión del botón pasa por ahí.
}

void JoinCodeScreen::updateJoin() { ok_->setEnabled(canJoin()); }

void JoinCodeScreen::onJoin() {
	if (!canJoin()) {
		return;
	}
	joining_ = true;
	showState(ScreenState::Loading);
	sessionService_->joinGame(code_->text().trimmed().toStdString(), username());
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
	// El esperado: sintaxis válida para el formato. Que el código exista
	// lo dice el servidor (ServiceError::InvalidCode).
	return !joining_ && codeFormat_.isValid(
			code_->text().trimmed().toStdString());
}
