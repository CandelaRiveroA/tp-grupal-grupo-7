#include "CharacterScreen.h"

#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <vector>

#include "common/model/draft/GameSelection.h"
#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt/services/CharacterLayerSource.h"
#include "qt/services/SessionService.h"
#include "qt/theme/CharacterRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/theme/WorldSpriteRects.h"
#include "qt/widgets/CharacterAppearanceEditor.h"
#include "qt/widgets/InputBox.h"
#include "qt/widgets/TitleButton.h"
#include "qt/widgets/WorldSelectorWidget.h"

CharacterScreen::CharacterScreen(SessionService* session,
				 const CharacterLayerSource* source, QWidget* parent,
				 core::MusicControl* music)
	: Screen(parent)
	, session_(session)
	, music_(music)
	, source_(source)
	, editor_(nullptr)
	, content_(nullptr)
	, worldLabel_(nullptr)
	, worldSelector_(nullptr)
	, characterName_(nullptr)
	, gameName_(nullptr)
	, create_(nullptr)
	, backButton_(nullptr)
	, status_(nullptr)
	, errorMessages_() {
	buildMessages();
	editor_ = new CharacterAppearanceEditor(source_, sprites_, this);
	connect(editor_, &CharacterAppearanceEditor::appearanceChanged, this,
		&CharacterScreen::updateValidation);
	buildUi();

	connect(session_, &SessionService::gameCreated, this,
		&CharacterScreen::onCreated);
	connect(session_, &SessionService::gameJoined, this,
		&CharacterScreen::onJoined);
	connect(session_, &SessionService::sessionFailed, this,
		&CharacterScreen::onSessionFailed);

	resetDraft();
}

void CharacterScreen::startNewGame(
	const std::vector<WorldSummary>& worlds) {
	mode_ = Mode::NewGame;
	worldSelector_->setWorlds(worlds);
	world_ = worldSelector_->selectedWorld();
	resetDraft();
}

void CharacterScreen::onWorldSelected(WorldSummary world) {
	if (mode_ != Mode::NewGame) {
		return;
	}
	world_ = world;
	worldLabel_->setText(
		QString("Mundo: %1").arg(QString::fromStdString(world_->name)));
	updateValidation();
}

void CharacterScreen::startJoinGame(const std::string& gameName) {
	mode_ = Mode::JoinGame;
	selection_.gameName = gameName;
	world_ = std::nullopt;
	resetDraft();
}

void CharacterScreen::onEnter() {
	if (music_ != nullptr) {
		music_->play(core::MusicTrack::CharacterSelect);
	}

	characterName_->setFocus();
	Screen::onEnter();
}

void CharacterScreen::onExit() {
	// Devuelve la musica del lobby: las demas pantallas no piden pista.
	if (music_ != nullptr) {
		music_->play(core::MusicTrack::MenuTheme);
	}

	Screen::onExit();
	submitting_ = false;
}

void CharacterScreen::buildMessages() {
	errorMessages_[ServiceError::GameNameTaken] =
		"Ya existe una partida con ese nombre.";
	errorMessages_[ServiceError::InvalidCode] =
		"No hay ninguna partida con ese código.";
	errorMessages_[ServiceError::GameFull] = "La partida está llena.";
	errorMessages_[ServiceError::CharacterNameTaken] =
		"Ya hay un jugador con ese personaje en la partida.";
	errorMessages_[ServiceError::UsernameTaken] = "Ese usuario ya está conectado.";
	errorMessages_[ServiceError::Network] =
		"No se pudo conectar con el servidor.";
}

void CharacterScreen::buildUi() {
	setObjectName("character");

	auto* root = new QVBoxLayout(this);
	root->setContentsMargins(48, 24, 28, 12);
	root->setSpacing(8);

	// Ventana con toda la configuración, centrada sobre la pantalla.
	// El panel de madera es un QLabel con el pixmap
	auto* frame = new QLabel(this);
	frame->setPixmap(sprites_.windowBox());
	frame->setFixedSize(CHAR_WINDOW_W, CHAR_WINDOW_H);
	frame->setObjectName("characterWindow");
	content_ = frame;
	auto* window = new QVBoxLayout(content_);
	window->setContentsMargins(CHAR_WINDOW_PAD, CHAR_WINDOW_PAD,
				   CHAR_WINDOW_PAD, CHAR_WINDOW_PAD - 4);
	window->setSpacing(16);

	auto* columns = new QHBoxLayout();
	columns->setSpacing(CHAR_COLUMN_GAP);
	columns->addWidget(editor_->previewColumn(this), 0, Qt::AlignTop);
	columns->addWidget(makeFormColumn(), 0, Qt::AlignTop);
	window->addLayout(columns, 1);

	create_ = new TitleButton(sprites_.okButton(), sprites_.okButtonHover(),
				  "Crear", content_);
	create_->setObjectName("createButton");
	create_->setEnabled(false);
	connect(create_, &QAbstractButton::clicked, this,
		&CharacterScreen::submit);
	auto* createRow = new QHBoxLayout();
	createRow->addStretch();
	createRow->addWidget(create_);
	window->addLayout(createRow);

	worldSelector_ = new WorldSelectorWidget(this);
	connect(worldSelector_, &WorldSelectorWidget::worldSelected, this,
		&CharacterScreen::onWorldSelected);

	auto* centerRow = new QHBoxLayout();
	centerRow->setSpacing(WorldSpriteRects::kPanelGap);
	centerRow->addStretch();
	centerRow->addWidget(content_, 0, Qt::AlignTop);
	centerRow->addWidget(worldSelector_, 0, Qt::AlignTop);
	centerRow->addStretch();
	root->addStretch();
	root->addLayout(centerRow);
	root->addStretch();

	status_ = new QLabel(this);
	status_->setObjectName("screenStatus");
	status_->setAlignment(Qt::AlignCenter);
	status_->setWordWrap(true);
	status_->setVisible(false);
	root->addWidget(status_);

	auto* buttons = new QHBoxLayout();
	
	TitleSprites sprites;
	backButton_ = new TitleButton(sprites.backButton(),
				      sprites.backButtonHover(), "Volver", this);
	backButton_->setObjectName("backButton");
	backButton_->setText("Volver");
	connect(backButton_, &QAbstractButton::clicked, this, [this]() {
		emit requestNavigate(mode_ == Mode::JoinGame
					      ? ScreenId::JoinCode
					      : ScreenId::MainMenu);
	});
	buttons->addStretch();
	buttons->addWidget(backButton_);
	root->addLayout(buttons);
}

QLineEdit* CharacterScreen::makeInputRow(QVBoxLayout* layout,
					 const QString& label,
					 const QString& objectName, int maxLength) {
	QWidget* parent = layout->parentWidget();
	auto* field = new QVBoxLayout();
	field->setSpacing(CHAR_INPUT_LABEL_GAP);
	field->addWidget(new QLabel(label, parent));
	auto* box = new InputBox(
		sprites_.inputBox(),
		QMargins(CHAR_INPUT_PAD_LEFT, CHAR_INPUT_PAD_TOP,
			 CHAR_INPUT_PAD_RIGHT, CHAR_INPUT_PAD_BOTTOM),
		parent);
	QLineEdit* edit = box->edit();
	edit->setObjectName(objectName);
	edit->setMaxLength(maxLength);
	field->addWidget(box, 0, Qt::AlignLeft);
	layout->addLayout(field);
	return edit;
}

QWidget* CharacterScreen::makeFormColumn() {
	auto* column = new QWidget(this);
	column->setObjectName("formColumn");
	column->setFixedWidth(CHAR_FORM_COL_W);
	auto* layout = new QVBoxLayout(column);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	characterName_ = makeInputRow(layout, "Nombre", "characterInput",
				      CHAR_INPUT_MAX_CHARS);
	gameName_ = makeInputRow(layout, "Nombre de la granja", "gameInput",
				 CHAR_INPUT_MAX_CHARS);

	worldLabel_ = new QLabel(column);
	worldLabel_->setObjectName("worldLabel");
	layout->addWidget(worldLabel_);

	layout->addSpacing(16);
	layout->addWidget(editor_->sliderColumn(column));
	layout->addStretch();

	connect(characterName_, &QLineEdit::textChanged, this,
		[this](const QString& text) {
			selection_.characterName = text.trimmed().toStdString();
			updateValidation();
		});
	connect(gameName_, &QLineEdit::textChanged, this,
		[this](const QString& text) {
			selection_.gameName = text.trimmed().toStdString();
			updateValidation();
		});
	connect(characterName_, &QLineEdit::returnPressed, this,
		&CharacterScreen::submit);
	connect(gameName_, &QLineEdit::returnPressed, this,
		&CharacterScreen::submit);
	return column;
}

void CharacterScreen::updateValidation() {
	create_->setEnabled(!submitting_ && canSubmit());
}

void CharacterScreen::submit() {
	if (submitting_ || !canSubmit()) {
		return;
	}

	if (mode_ == Mode::JoinGame) {
		submitting_ = true;
		showState(ScreenState::Loading, "Uniéndose a la partida...");
		selection_.appearance = editor_->appearance();
		session_->joinGame(selection_.gameName, username(), selection_.characterName,
				   selection_.appearance);
		return;
	}

	if (!world_.has_value()) {
		return;
	}

	submitting_ = true;
	showState(ScreenState::Loading, "Creando partida...");

	selection_.appearance = editor_->appearance();
	selection_.worldName = world_->name;

	GameSelection game;
	game.username = username();
	game.gameName = selection_.gameName;
	game.characterName = selection_.characterName;
	game.worldName = world_->name;
	game.appearance = selection_.appearance;
	session_->createGame(game);
}

void CharacterScreen::onCreated() {
	if (!submitting_) {
		// Llegó después de salir de la pantalla: no se entra al juego.
		return;
	}
	submitting_ = false;
	emit characterConfirmed(selection_);
}

void CharacterScreen::onJoined(const std::string&, const std::string& worldName) {
	if (!submitting_) {
		return;
	}
	submitting_ = false;
	selection_.worldName = worldName;
	emit characterConfirmed(selection_);
}

void CharacterScreen::onSessionFailed(ServiceError error) {
	if (!submitting_) {
		// Falló otro pedido (unirse, cargar): no es error de esta
		// pantalla.
		return;
	}
	submitting_ = false;
	showState(ScreenState::Error, messageFor(error));
}

void CharacterScreen::resetDraft() {
	// En modo unirse el nombre de la partida lo puso el server: fuera del
	// draft, queda como info read-only de a qué partida entraste. La
	// granja no se customiza (el enunciado: "sin poder customizar la
	// granja"): sin campo editable y sin rastro del mundo.
	const std::string joinedName =
		mode_ == Mode::JoinGame ? selection_.gameName : std::string();
	selection_ = CharacterSelection{};
	submitting_ = false;

	characterName_->clear();
	gameName_->clear();

	gameName_->setReadOnly(mode_ == Mode::JoinGame);
	gameName_->style()->unpolish(gameName_);
	gameName_->style()->polish(gameName_);
	worldLabel_->setVisible(mode_ != Mode::JoinGame);
	worldSelector_->setVisible(mode_ == Mode::NewGame);
	if (mode_ == Mode::JoinGame) {
		selection_.gameName = joinedName;
		const QSignalBlocker blocker(gameName_);
		gameName_->setText(QString::fromStdString(joinedName));
	}

	if (world_.has_value()) {
		worldLabel_->setText(
			QString("Mundo: %1")
				.arg(QString::fromStdString(world_->name)));
	} else {
		worldLabel_->clear();
	}

	create_->setText(mode_ == Mode::JoinGame ? "Aceptar" : "Crear");
	create_->setToolTip(create_->text());

	showState(ScreenState::Ready, QString());
	editor_->setAppearance(selection_.appearance);
}

void CharacterScreen::showState(ScreenState state, const QString& message) {
	const bool busy = state == ScreenState::Loading;

	content_->setEnabled(!busy);
	worldSelector_->setEnabled(!busy);
	backButton_->setEnabled(!busy);
	create_->setEnabled(!busy && canSubmit());

	status_->setVisible(state != ScreenState::Ready);
	if (state != ScreenState::Ready) {
		status_->setText(message);
	}
}

bool CharacterScreen::canSubmit() const {
	if (mode_ == Mode::JoinGame) {
		// Solo el personaje: la partida y el mundo ya están definidos.
		return !selection_.characterName.empty();
	}
	return world_.has_value() && !selection_.characterName.empty() &&
	       !selection_.gameName.empty();
}

QString CharacterScreen::messageFor(ServiceError error) const {
	const auto message = errorMessages_.find(error);
	if (message == errorMessages_.end()) {
		return "No se pudo crear la partida.";
	}
	return message->second;
}
