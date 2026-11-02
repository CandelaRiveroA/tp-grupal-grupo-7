#include "CharacterScreen.h"

#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyle>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QWidget>

#include "common/model/draft/GameSelection.h"
#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt/services/CharacterLayerSource.h"
#include "qt/services/SessionService.h"
#include "qt/theme/CharacterRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/util/IndexCycle.h"
#include "qt/widgets/CharacterPreview.h"
#include "qt/widgets/CycleRow.h"
#include "qt/widgets/InputBox.h"
#include "qt/widgets/SliderRow.h"
#include "qt/widgets/TitleButton.h"

CharacterScreen::CharacterScreen(SessionService* session,
				 const CharacterLayerSource* source, QWidget* parent,
				 core::MusicControl* music)
	: Screen(parent)
	, session_(session)
	, music_(music)
	, source_(source)
	, layers_()
	, content_(nullptr)
	, preview_(nullptr)
	, worldLabel_(nullptr)
	, characterName_(nullptr)
	, gameName_(nullptr)
	, create_(nullptr)
	, backButton_(nullptr)
	, status_(nullptr)
	, cycleRows_()
	, sliderRows_()
	, errorMessages_() {
	buildMessages();
	buildUi();

	connect(session_, &SessionService::gameCreated, this,
		&CharacterScreen::onCreated);
	connect(session_, &SessionService::sessionFailed, this,
		&CharacterScreen::onSessionFailed);

	resetDraft();
}

void CharacterScreen::startNewGame(const WorldSummary& world) {
	mode_ = Mode::NewGame;
	world_ = world;
	resetDraft();
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
	if (!cloudTimer_.isActive()) {
		cloudTimer_.start(CLOUD_ANIMATION_TICK_MS, this);
	}
}

void CharacterScreen::onExit() {
	// Devuelve la musica del lobby: las demas pantallas no piden pista.
	if (music_ != nullptr) {
		music_->play(core::MusicTrack::MenuTheme);
	}

	if (cloudTimer_.isActive()) {
		cloudTimer_.stop();
	}
	submitting_ = false;
}

void CharacterScreen::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	backdrop_.paint(painter);
	QWidget::paintEvent(event);
}

void CharacterScreen::timerEvent(QTimerEvent* event) {
	if (event->timerId() == cloudTimer_.timerId()) {
		backdrop_.step(float(CLOUD_ANIMATION_TICK_MS) / 1000.0f);
		update();
		return;
	}
	QWidget::timerEvent(event);
}


void CharacterScreen::buildMessages() {
	errorMessages_[ServiceError::GameNameTaken] =
		"Ya existe una partida con ese nombre.";
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
	columns->addWidget(makePreviewColumn(), 0, Qt::AlignTop);
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

	root->addStretch();
	root->addWidget(content_, 0, Qt::AlignHCenter);
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
					      : ScreenId::WorldSelect);
	});
	buttons->addStretch();
	buttons->addWidget(backButton_);
	root->addLayout(buttons);
}

QWidget* CharacterScreen::makePreviewColumn() {
	auto* column = new QWidget(this);
	column->setObjectName("previewColumn");
	column->setFixedWidth(CHAR_PREVIEW_COL_W);
	auto* layout = new QVBoxLayout(column);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(CHAR_ROW_GAP);

	preview_ = new CharacterPreview(source_, column);
	preview_->setFixedSize(CHAR_PREVIEW_W, CHAR_PREVIEW_H);
	preview_->setBackground(sprites_.portraitBackground());
	layout->addWidget(preview_, 0, Qt::AlignHCenter);

	struct Row {
		CharacterLayer layer;
		const char* name;
	};
	const Row rows[] = {
		{CharacterLayer::Skin, "Piel"},
		{CharacterLayer::Hair, "Pelo"},
		{CharacterLayer::Shirt, "Camisa"},
		{CharacterLayer::Pants, "Pantalón"},
		{CharacterLayer::Accessory, "Accesorio"},
	};
	for (const Row& row : rows) {
		layout->addWidget(makeRow(row.layer, row.name));
	}
	return column;
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
	layout->addWidget(makeSlider(CharacterLayer::Eyes, "Color de ojos", "eyesSlider"));
	layout->addWidget(
		makeSlider(CharacterLayer::HairColor, "Color de pelo",
			   "hairColorSlider"));
	layout->addWidget(makeSlider(CharacterLayer::PantsColor,
				     "Color de pantalón", "pantsColorSlider"));
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

QWidget* CharacterScreen::makeRow(CharacterLayer layer, const QString& name) {
	auto* row = new CycleRow(name, sprites_.leftArrow(),
				 sprites_.leftArrowHover(),
				 sprites_.rightArrow(),
				 sprites_.rightArrowHover(), this);
	connect(row, &CycleRow::stepRequested, this,
		[this, layer](int direction) { cycleLayer(layer, direction); });
	cycleRows_[layer] = row;
	return row;
}

QWidget* CharacterScreen::makeSlider(CharacterLayer layer,
				     const QString& name,
				     const QString& objectName) {
	const int total = source_->count(layer);
	auto* row = new SliderRow(name, objectName, total > 0 ? total - 1 : 0,
				  this);
	connect(row, &SliderRow::valueChanged, this, [this, layer](int value) {
		setLayer(layer, static_cast<std::uint8_t>(value));
	});
	sliderRows_[layer] = row;
	return row;
}

void CharacterScreen::cycleLayer(CharacterLayer layer, int direction) {
	const std::uint8_t total = source_->count(layer);
	if (total == 0) {
		return;
	}

	const IndexCycle cycle(total);
	const std::uint8_t current =
		layers_.value(selection_.appearance, layer);
	const std::uint8_t next = direction > 0 ? cycle.next(current)
					       : cycle.previous(current);
	setLayer(layer, next);
}

void CharacterScreen::setLayer(CharacterLayer layer, std::uint8_t index) {
	const std::uint8_t total = source_->count(layer);
	if (total == 0) {
		return;
	}
	if (index >= total) {
		index = static_cast<std::uint8_t>(total - 1);
	}

	layers_.set(selection_.appearance, layer, index);
	applyDraft();
}

void CharacterScreen::applyDraft() {
	preview_->setAppearance(selection_.appearance);

	for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count);
	     ++layer) {
		const auto current = static_cast<CharacterLayer>(layer);
		const std::uint8_t value =
			layers_.value(selection_.appearance, current);

		const auto cycle = cycleRows_.find(current);
		if (cycle != cycleRows_.end()) {
			cycle->second->setValue(QString::number(value));
		}

		const auto slider = sliderRows_.find(current);
		if (slider != sliderRows_.end()) {
			slider->second->setValue(value);
		}
	}
	updateValidation();
}

void CharacterScreen::updateValidation() {
	create_->setEnabled(!submitting_ && canSubmit());
}

void CharacterScreen::submit() {
	if (submitting_ || !canSubmit()) {
		return;
	}

	if (mode_ == Mode::JoinGame) {
		// La sesión ya se unió al aceptar el código: acá solo queda
		// confirmar el personaje. TODO con protocolo: confirmar cómo
		// viaja el personaje del joiner (ver
		// docs/ui/plan-e2-servicios.md).
		emit characterConfirmed(selection_);
		return;
	}

	if (!world_.has_value()) {
		return;
	}

	submitting_ = true;
	showState(ScreenState::Loading, "Creando partida...");

	GameSelection game;
	game.gameName = selection_.gameName;
	game.characterName = selection_.characterName;
	game.worldId = world_->id;
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
	applyDraft();
}

void CharacterScreen::showState(ScreenState state, const QString& message) {
	const bool busy = state == ScreenState::Loading;

	content_->setEnabled(!busy);
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
