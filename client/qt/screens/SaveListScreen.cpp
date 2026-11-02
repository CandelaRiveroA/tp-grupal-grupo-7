#include "SaveListScreen.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <utility>

#include "qt/services/CharacterLayerSource.h"
#include "qt/services/SaveService.h"
#include "qt/services/SessionService.h"
#include "qt/widgets/CharacterPreview.h"

SaveListScreen::SaveListScreen(SaveService* saves, SessionService* sessions,
			       const CharacterLayerSource* source, QWidget* parent)
	: Screen(parent)
	, saveService_(saves)
	, sessionService_(sessions)
	, source_(source)
	, scroll_(nullptr)
	, rowsHost_(nullptr)
	, rowsLayout_(nullptr)
	, group_(nullptr)
	, status_(nullptr)
	, retry_(nullptr)
	, states_()
	, errorMessages_() {
	setObjectName("saveList");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(64, 48, 64, 48);
	layout->setSpacing(16);

	auto* title = new QLabel("Elegí una partida", this);
	title->setObjectName("screenTitle");
	layout->addWidget(title);

	scroll_ = new QScrollArea(this);
	scroll_->setWidgetResizable(true);
	scroll_->setFrameShape(QFrame::NoFrame);
	scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_->setVisible(false);
	layout->addWidget(scroll_, 1);

	rowsHost_ = new QWidget(scroll_);
	rowsLayout_ = new QVBoxLayout(rowsHost_);
	rowsLayout_->setContentsMargins(0, 0, 0, 0);
	rowsLayout_->setSpacing(16);
	scroll_->setWidget(rowsHost_);

	group_ = new QButtonGroup(this);
	connect(group_, &QButtonGroup::idClicked, this,
		&SaveListScreen::onRowClicked);

	status_ = new QLabel(this);
	status_->setObjectName("screenStatus");
	status_->setAlignment(Qt::AlignCenter);
	status_->setWordWrap(true);
	status_->setVisible(false);
	layout->addWidget(status_);

	retry_ = new QPushButton("Reintentar", this);
	retry_->setObjectName("retryButton");
	retry_->setVisible(false);
	connect(retry_, &QPushButton::clicked, this,
		&SaveListScreen::requestSaves);
	layout->addWidget(retry_, 0, Qt::AlignHCenter);

	auto* buttons = new QHBoxLayout();
	buttons->addStretch();

	auto* back = new QPushButton("Volver", this);
	back->setObjectName("backButton");
	connect(back, &QPushButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::MainMenu);
	});
	buttons->addWidget(back);

	layout->addLayout(buttons);

	connect(saveService_, &SaveService::savesReady, this,
		&SaveListScreen::showSaves);
	connect(saveService_, &SaveService::savesFailed, this,
		&SaveListScreen::showError);
	connect(sessionService_, &SessionService::gameLoaded, this,
		&SaveListScreen::onGameLoaded);
	connect(sessionService_, &SessionService::sessionFailed, this,
		&SaveListScreen::onSessionFailed);

	states_.bind(ScreenState::Ready, scroll_);
	states_.bind(ScreenState::Loading, status_);
	states_.bind(ScreenState::Empty, status_);
	states_.bind(ScreenState::Error, status_);
	states_.bind(ScreenState::Error, retry_);

	buildMessages();
}

void SaveListScreen::onEnter() {
	// Siempre se vuelve a pedir: el server puede haber creado o borrado
	// partidas desde la última visita.
	requestSaves();
}

void SaveListScreen::buildMessages() {
	errorMessages_[ServiceError::Network] =
		"No se pudo conectar con el servidor.";
	errorMessages_[ServiceError::SaveNotFound] =
		"La partida ya no existe.";
}

void SaveListScreen::requestSaves() {
	if (loading_) {
		return;
	}
	loading_ = true;
	showState(ScreenState::Loading, "Buscando partidas…");
	saveService_->requestSaves();
}

void SaveListScreen::showSaves(std::vector<SaveSummary> saves) {
	loading_ = false;
	saves_ = std::move(saves);
	selectedSave_ = -1;
	clearRows();

	const int count = static_cast<int>(saves_.size());
	for (int i = 0; i < count; ++i) {
		rowsLayout_->addWidget(makeRow(saves_[i], i));
	}

	if (saves_.empty()) {
		showState(ScreenState::Empty, "No hay partidas guardadas.");
		return;
	}
	showState(ScreenState::Ready, QString());
}

void SaveListScreen::showError(ServiceError error) {
	loading_ = false;
	showState(ScreenState::Error, messageFor(error));
}

void SaveListScreen::showState(ScreenState state, const QString& message) {
	if (state != ScreenState::Ready) {
		status_->setText(message);
	}
	states_.setState(state);
}

void SaveListScreen::clearRows() {
	while (QLayoutItem* item = rowsLayout_->takeAt(0)) {
		delete item->widget();
		delete item;
	}
}

QWidget* SaveListScreen::makeRow(const SaveSummary& save, int index) {
	auto* row = new QPushButton(rowsHost_);
	row->setObjectName("saveRow");
	row->setCursor(Qt::PointingHandCursor);
	row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	auto* line = new QHBoxLayout(row);
	line->setContentsMargins(16, 12, 16, 12);
	line->setSpacing(24);

	// Retrato a escala 2 del lienzo (el sizeHint del preview apunta a la
	// escala 6 de la pantalla de personaje).
	auto* portrait = new CharacterPreview(source_, row);
	portrait->setAttribute(Qt::WA_TransparentForMouseEvents);
	portrait->setAppearance(save.players.empty() ? CharacterAppearance{}
						     : save.players.front()
							       .appearance);
	const QSize portraitSize = portrait->sizeHint() / 3;
	portrait->setFixedSize(portraitSize);
	line->addWidget(portrait, 0, Qt::AlignVCenter);

	// El botón no toma la altura de su layout: la fijamos desde el
	// retrato (12 de padding arriba y abajo).
	row->setFixedHeight(portraitSize.height() + 24);

	auto* info = new QVBoxLayout();
	info->setSpacing(4);

	auto* name = new QLabel(QString::fromStdString(save.gameName), row);
	name->setObjectName("saveName");
	name->setAttribute(Qt::WA_TransparentForMouseEvents);
	info->addWidget(name);

	if (!save.players.empty()) {
		auto* player =
			new QLabel(QString::fromStdString(
					   save.players.front().name),
				   row);
		player->setObjectName("savePlayer");
		player->setAttribute(Qt::WA_TransparentForMouseEvents);
		info->addWidget(player);
	}

	if (!save.inGameDate.empty()) {
		auto* date =
			new QLabel(QString::fromStdString(save.inGameDate),
				   row);
		date->setObjectName("saveDate");
		date->setAttribute(Qt::WA_TransparentForMouseEvents);
		info->addWidget(date);
	}

	if (save.multiplayer) {
		auto* badge = new QLabel("Cooperativa", row);
		badge->setObjectName("saveBadge");
		badge->setAttribute(Qt::WA_TransparentForMouseEvents);
		info->addWidget(badge);
	}

	line->addLayout(info, 1);
	group_->addButton(row, index);
	return row;
}

void SaveListScreen::onRowClicked(int index) {
	if (selecting_ || index < 0 || index >= static_cast<int>(saves_.size()))
	{
		return;
	}
	selectedSave_ = index;
	selecting_ = true;
	showState(ScreenState::Loading, "Cargando partida…");
	sessionService_->loadGame(saves_[selectedSave_].id);
}

void SaveListScreen::onGameLoaded() {
	if (!selecting_ || selectedSave_ < 0 ||
	    selectedSave_ >= static_cast<int>(saves_.size())) {
		return;
	}
	selecting_ = false;
	emit saveLoaded(saves_[selectedSave_]);
}

void SaveListScreen::onSessionFailed(ServiceError error) {
	if (!selecting_) {
		// Falló otro pedido (crear, unirse): no es error de esta
		// pantalla.
		return;
	}
	selectedSave_ = -1;
	selecting_ = false;
	showState(ScreenState::Error, messageFor(error));
}

QString SaveListScreen::messageFor(ServiceError error) const {
	const auto message = errorMessages_.find(error);
	if (message == errorMessages_.end()) {
		return "No se pudieron abrir las partidas.";
	}
	return message->second;
}
