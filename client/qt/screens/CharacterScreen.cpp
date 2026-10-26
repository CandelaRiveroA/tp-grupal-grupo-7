#include "CharacterScreen.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include "common/model/draft/GameSelection.h"
#include "qt/services/AssetCatalog.h"
#include "qt/services/SessionService.h"
#include "qt/util/IndexCycle.h"
#include "qt/widgets/CharacterPreview.h"

CharacterScreen::CharacterScreen(SessionService* session,
				 const AssetCatalog* catalog, QWidget* parent)
	: Screen(parent)
	, session_(session)
	, catalog_(catalog)
	, layers_()
	, content_(nullptr)
	, preview_(nullptr)
	, worldLabel_(nullptr)
	, characterName_(nullptr)
	, gameName_(nullptr)
	, create_(nullptr)
	, backButton_(nullptr)
	, status_(nullptr)
	, values_()
	, sliders_()
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
	world_ = world;
	resetDraft();
}

void CharacterScreen::onEnter() { characterName_->setFocus(); }

void CharacterScreen::buildMessages() {
	errorMessages_[ServiceError::GameNameTaken] =
		"Ya existe una partida con ese nombre.";
	errorMessages_[ServiceError::Network] =
		"No se pudo conectar con el servidor.";
}

void CharacterScreen::buildUi() {
	setObjectName("character");

	auto* root = new QVBoxLayout(this);
	root->setContentsMargins(48, 24, 48, 24);
	root->setSpacing(8);

	// Ventana con toda la configuración, centrada sobre la pantalla.
	content_ = new QFrame(this);
	content_->setObjectName("characterWindow");
	content_->setMaximumWidth(kWindowMaxWidth);
	auto* window = new QVBoxLayout(content_);
	window->setContentsMargins(24, 24, 24, 24);
	window->setSpacing(16);

	auto* columns = new QHBoxLayout();
	columns->setSpacing(48);
	columns->addWidget(makePreviewColumn(), 0, Qt::AlignTop);
	columns->addWidget(makeFormColumn(), 1, Qt::AlignTop);
	window->addLayout(columns, 1);

	create_ = new QPushButton("Crear", content_);
	create_->setObjectName("createButton");
	create_->setDefault(true);
	create_->setEnabled(false);
	connect(create_, &QPushButton::clicked, this,
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
	backButton_ = new QPushButton("Volver", this);
	backButton_->setObjectName("backButton");
	connect(backButton_, &QPushButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::WorldSelect);
	});
	buttons->addStretch();
	buttons->addWidget(backButton_);
	root->addLayout(buttons);
}

QWidget* CharacterScreen::makePreviewColumn() {
	auto* column = new QWidget(this);
	column->setObjectName("previewColumn");
	auto* layout = new QVBoxLayout(column);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	preview_ = new CharacterPreview(catalog_, column);
	preview_->setFixedSize(preview_->sizeHint());
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
	layout->addStretch();
	return column;
}

QWidget* CharacterScreen::makeFormColumn() {
	auto* column = new QWidget(this);
	column->setObjectName("formColumn");
	auto* layout = new QVBoxLayout(column);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto* nameRow = new QHBoxLayout();
	auto* nameLabel = new QLabel("Nombre", column);
	nameLabel->setMinimumWidth(110);
	nameRow->addWidget(nameLabel);
	characterName_ = new QLineEdit(column);
	characterName_->setMaxLength(16);
	characterName_->setMaximumWidth(420);
	characterName_->setPlaceholderText("Nombre del personaje");
	nameRow->addWidget(characterName_, 1);
	layout->addLayout(nameRow);

	auto* gameRow = new QHBoxLayout();
	auto* gameLabel = new QLabel("Partida", column);
	gameLabel->setMinimumWidth(110);
	gameRow->addWidget(gameLabel);
	gameName_ = new QLineEdit(column);
	gameName_->setMaxLength(20);
	gameName_->setMaximumWidth(420);
	gameName_->setPlaceholderText("Nombre de la partida");
	gameRow->addWidget(gameName_, 1);
	layout->addLayout(gameRow);

	worldLabel_ = new QLabel(column);
	layout->addWidget(worldLabel_);

	layout->addSpacing(16);
	layout->addWidget(makeSlider(CharacterLayer::Eyes, "Color de ojos"));
	layout->addWidget(
		makeSlider(CharacterLayer::HairColor, "Color de pelo"));
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
	return column;
}

QWidget* CharacterScreen::makeRow(CharacterLayer layer, const QString& name) {
	auto* row = new QWidget(this);
	auto* layout = new QVBoxLayout(row);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	auto* line = new QHBoxLayout();
	line->setSpacing(8);

	auto* previous = new QPushButton("◀", row);
	previous->setFixedWidth(36);
	connect(previous, &QPushButton::clicked, this, [this, layer]() {
		cycleLayer(layer, -1);
	});
	line->addWidget(previous);

	auto* label = new QLabel(name, row);
	label->setAlignment(Qt::AlignCenter);
	label->setMinimumWidth(120);
	line->addWidget(label, 1);

	auto* next = new QPushButton("▶", row);
	next->setFixedWidth(36);
	connect(next, &QPushButton::clicked, this, [this, layer]() {
		cycleLayer(layer, 1);
	});
	line->addWidget(next);
	layout->addLayout(line);

	auto* value = new QLabel(row);
	value->setAlignment(Qt::AlignCenter);
	layout->addWidget(value);
	values_[layer] = value;
	return row;
}

QWidget* CharacterScreen::makeSlider(CharacterLayer layer,
				     const QString& name) {
	auto* block = new QWidget(this);
	auto* layout = new QVBoxLayout(block);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(2);

	layout->addWidget(new QLabel(name, block));

	auto* line = new QHBoxLayout();
	line->setSpacing(12);

	auto* slider = new QSlider(Qt::Horizontal, block);
	const int total = catalog_->count(layer);
	slider->setRange(0, total > 0 ? total - 1 : 0);
	slider->setSingleStep(1);
	slider->setPageStep(1);
	connect(slider, &QSlider::valueChanged, this,
		[this, layer](int value) {
			setLayer(layer, static_cast<std::uint8_t>(value));
		});
	line->addWidget(slider, 1);

	auto* value = new QLabel(block);
	value->setMinimumWidth(24);
	value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	line->addWidget(value);
	layout->addLayout(line);

	sliders_[layer] = slider;
	values_[layer] = value;
	return block;
}

void CharacterScreen::cycleLayer(CharacterLayer layer, int direction) {
	const std::uint8_t total = catalog_->count(layer);
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
	const std::uint8_t total = catalog_->count(layer);
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

		const auto label = values_.find(current);
		if (label != values_.end()) {
			label->second->setText(QString::number(value));
		}

		const auto slider = sliders_.find(current);
		if (slider != sliders_.end()) {
			const QSignalBlocker blocker(slider->second);
			slider->second->setValue(value);
		}
	}
	updateValidation();
}

void CharacterScreen::updateValidation() {
	create_->setEnabled(!submitting_ && canSubmit());
}

void CharacterScreen::submit() {
	if (submitting_ || !canSubmit() || !world_.has_value()) {
		return;
	}

	submitting_ = true;
	showState(ScreenState::Loading, "Creando partida…");

	GameSelection game;
	game.gameName = selection_.gameName;
	game.characterName = selection_.characterName;
	game.worldId = world_->id;
	game.appearance = selection_.appearance;
	session_->createGame(game);
}

void CharacterScreen::onCreated() {
	submitting_ = false;
	emit characterConfirmed(selection_);
}

void CharacterScreen::onSessionFailed(ServiceError error) {
	submitting_ = false;
	showState(ScreenState::Error, messageFor(error));
}

void CharacterScreen::resetDraft() {
	selection_ = CharacterSelection{};
	submitting_ = false;

	characterName_->clear();
	gameName_->clear();

	if (world_.has_value()) {
		worldLabel_->setText(
			QString("Mundo: %1")
				.arg(QString::fromStdString(world_->name)));
	} else {
		worldLabel_->clear();
	}

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
