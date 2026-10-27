#include "JoinCodeScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

#include "qt/services/SessionService.h"

JoinCodeScreen::JoinCodeScreen(SessionService* sessions, QWidget* parent)
	: Screen(parent)
	, sessionService_(sessions)
	, code_(nullptr)
	, ok_(nullptr)
	, back_(nullptr)
	, status_(nullptr)
	, errorMessages_() {
	setObjectName("joinCode");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(96, 64, 96, 64);

	layout->addStretch();

	auto* prompt = new QLabel("Ingresá el código de invitación…", this);
	prompt->setObjectName("codePrompt");
	prompt->setAlignment(Qt::AlignCenter);
	layout->addWidget(prompt);

	auto* row = new QHBoxLayout();
	row->setSpacing(16);
	code_ = new QLineEdit(this);
	code_->setObjectName("codeInput");
	code_->setMinimumWidth(320);
	ok_ = new QPushButton("OK", this);
	ok_->setObjectName("okButton");
	ok_->setEnabled(false);
	row->addStretch();
	row->addWidget(code_);
	row->addWidget(ok_);
	row->addStretch();
	layout->addLayout(row);

	status_ = new QLabel(this);
	status_->setObjectName("screenStatus");
	status_->setAlignment(Qt::AlignCenter);
	status_->setWordWrap(true);
	status_->setVisible(false);
	layout->addWidget(status_);

	layout->addStretch();

	auto* buttons = new QHBoxLayout();
	back_ = new QPushButton("Volver", this);
	back_->setObjectName("backButton");
	connect(back_, &QPushButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::MainMenu);
	});
	buttons->addStretch();
	buttons->addWidget(back_);
	layout->addLayout(buttons);

	connect(code_, &QLineEdit::textChanged, this,
		&JoinCodeScreen::updateJoin);
	connect(ok_, &QPushButton::clicked, this, &JoinCodeScreen::onJoin);
	connect(sessionService_, &SessionService::gameJoined, this,
		&JoinCodeScreen::onGameJoined);
	connect(sessionService_, &SessionService::sessionFailed, this,
		&JoinCodeScreen::onSessionFailed);

	buildMessages();
}

void JoinCodeScreen::onEnter() { code_->setFocus(); }

void JoinCodeScreen::onExit() {
	joining_ = false;
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
		status_->setText("Conectando…");
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
