#include "SaveListScreen.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <utility>

#include "qt/screens/Screen.h"
#include "qt/theme/SaveListRects.h"
#include "qt/theme/TitleSprites.h"
#include "qt/services/SaveService.h"
#include "qt/services/SessionService.h"
#include "qt/widgets/SaveRowButton.h"
#include "qt_theme/TitleButton.h"

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
	, window_(nullptr)
	, scrollUp_(nullptr)
	, scrollDown_(nullptr)
	, backButton_(nullptr)
	, states_()
	, sprites_()
	, errorMessages_() {
	setObjectName("saveList");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(SAVE_ROOT_MARGIN_LEFT, SAVE_ROOT_MARGIN_TOP,
				   SAVE_ROOT_MARGIN_RIGHT, SAVE_ROOT_MARGIN_BOTTOM);
	layout->setSpacing(0);

	layout->addStretch(SAVE_STRETCH_TOP);

	auto* centerRow = new QHBoxLayout();
	centerRow->addStretch(SAVE_STRETCH_LEFT);

	window_ = new QLabel(this);
	window_->setObjectName("saveWindow");
	window_->setPixmap(sprites_.windowBox());
	window_->setFixedSize(SAVE_WINDOW_W, SAVE_WINDOW_H);
	centerRow->addWidget(window_);

	auto* windowLayout = new QVBoxLayout(window_);
	windowLayout->setContentsMargins(SAVE_WINDOW_PAD, SAVE_WINDOW_PAD,
					 SAVE_WINDOW_PAD, SAVE_WINDOW_PAD);

	scroll_ = new QScrollArea(window_);
	scroll_->setObjectName("saveScroll");
	scroll_->setWidgetResizable(true);
	scroll_->setFrameShape(QFrame::NoFrame);
	scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_->setAutoFillBackground(false);
	scroll_->viewport()->setAutoFillBackground(false);
	scroll_->setVisible(false);
	windowLayout->addWidget(scroll_, 1);

	rowsHost_ = new QWidget(scroll_);
	rowsHost_->setObjectName("saveRowsHost");
	rowsHost_->setAutoFillBackground(false);
	rowsLayout_ = new QVBoxLayout(rowsHost_);
	rowsLayout_->setContentsMargins(0, 0, 0, 0);
	rowsLayout_->setSpacing(0);
	rowsLayout_->setAlignment(Qt::AlignTop);
	scroll_->setWidget(rowsHost_);

	// lo apagamos para que la zona libre muestre el panel y no el Window gris.
	rowsHost_->setAutoFillBackground(false);

	group_ = new QButtonGroup(this);
	connect(group_, &QButtonGroup::idClicked, this,
		&SaveListScreen::onRowClicked);

	status_ = new QLabel(window_);
	status_->setObjectName("screenStatus");
	status_->setAlignment(Qt::AlignCenter);
	status_->setWordWrap(true);
	status_->setVisible(false);
	windowLayout->addWidget(status_);

	retry_ = new QPushButton("Reintentar", window_);
	retry_->setObjectName("retryButton");
	retry_->setVisible(false);
	connect(retry_, &QPushButton::clicked, this,
		&SaveListScreen::requestSaves);
	windowLayout->addWidget(retry_, 0, Qt::AlignHCenter);

	centerRow->addSpacing(SAVE_ARROWS_GAP);

	auto* arrows = new QVBoxLayout();
	scrollUp_ = new TitleButton(sprites_.upArrow(), sprites_.upArrowHover(),
				    "Subir", this);
	scrollUp_->setObjectName("scrollUpButton");
	scrollUp_->setFixedSize(SAVE_SCROLL_ARROW_W, SAVE_SCROLL_ARROW_H);
	scrollDown_ = new TitleButton(sprites_.downArrow(),
				      sprites_.downArrowHover(), "Bajar", this);
	scrollDown_->setObjectName("scrollDownButton");
	scrollDown_->setFixedSize(SAVE_SCROLL_ARROW_W, SAVE_SCROLL_ARROW_H);
	connect(scrollUp_, &QAbstractButton::clicked, this,
		&SaveListScreen::scrollUp);
	connect(scrollDown_, &QAbstractButton::clicked, this,
		&SaveListScreen::scrollDown);
	
	arrows->addSpacing(SAVE_SCROLL_ARROW_GAP_TOP);
	arrows->addWidget(scrollUp_);
	arrows->addStretch(1);
	arrows->addWidget(scrollDown_);
	arrows->addSpacing(SAVE_SCROLL_ARROW_GAP_BOTTOM);
	centerRow->addLayout(arrows);
	centerRow->addStretch(SAVE_STRETCH_RIGHT);

	layout->addLayout(centerRow);
	layout->addStretch(SAVE_STRETCH_BOTTOM);

	auto* buttons = new QHBoxLayout();
	TitleSprites titles;
	backButton_ = new TitleButton(titles.backButton(),
				      titles.backButtonHover(), "Volver", this);
	backButton_->setObjectName("backButton");
	backButton_->setText("Volver");
	connect(backButton_, &QAbstractButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::MainMenu);
	});
	buttons->addStretch();
	buttons->addWidget(backButton_);
	layout->addLayout(buttons);

	connect(saveService_, &SaveService::savesReady, this,
		&SaveListScreen::showSaves);
	connect(saveService_, &SaveService::savesFailed, this,
		&SaveListScreen::showError);
	connect(saveService_, &SaveService::saveDeleted, this,
		&SaveListScreen::onSaveDeleted);
	connect(saveService_, &SaveService::deleteFailed, this,
		&SaveListScreen::onDeleteFailed);
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

	connect(scroll_->verticalScrollBar(), &QScrollBar::valueChanged, this,
		&SaveListScreen::updateNavigation);
	connect(scroll_->verticalScrollBar(), &QScrollBar::rangeChanged, this,
		&SaveListScreen::updateNavigation);
	updateNavigation();
}

void SaveListScreen::onEnter() {
	// Siempre se vuelve a pedir: el server puede haber creado o borrado
	// partidas desde la última visita.
	Screen::onEnter();
	requestSaves();
}

void SaveListScreen::onExit() {
	loading_ = false;
	selecting_ = false;
	deleting_ = false;
	Screen::onExit();
	updateNavigation();
}

void SaveListScreen::buildMessages() {
	errorMessages_[ServiceError::Network] =
		"No se pudo conectar con el servidor.";
	errorMessages_[ServiceError::SaveNotFound] =
		"La partida ya no existe.";
}

void SaveListScreen::requestSaves() {
	if (loading_ || deleting_) {
		return;
	}
	loading_ = true;
	showState(ScreenState::Loading, "Buscando partidas...");
	saveService_->requestSaves(username());
}

void SaveListScreen::showSaves(std::vector<SaveSummary> saves) {
	loading_ = false;
	saves_ = std::move(saves);
	selectedSave_ = kNoSelection;
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
	updateNavigation();
}

void SaveListScreen::updateNavigation() {
	const bool busy = loading_ || selecting_ || deleting_;
	const bool scrollable =
		static_cast<int>(saves_.size()) > SAVE_VISIBLE_ROWS;
	QScrollBar* bar = scroll_->verticalScrollBar();

	// Se pinta en gris un QLabel con pixmap disabled y apaga el panel.
	scroll_->setEnabled(!busy);
	backButton_->setEnabled(!busy);
	scrollUp_->setEnabled(!busy && scrollable &&
			      bar->value() > bar->minimum());
	scrollDown_->setEnabled(!busy && scrollable &&
				bar->value() < bar->maximum());
}

void SaveListScreen::scrollUp() {
	QScrollBar* bar = scroll_->verticalScrollBar();
	bar->setValue(bar->value() - SAVE_ROW_H);
}

void SaveListScreen::scrollDown() {
	QScrollBar* bar = scroll_->verticalScrollBar();
	bar->setValue(bar->value() + SAVE_ROW_H);
}

void SaveListScreen::clearRows() {
	while (QLayoutItem* item = rowsLayout_->takeAt(0)) {
		delete item->widget();
		delete item;
	}
}

QWidget* SaveListScreen::makeRow(const SaveSummary& save, int index) {
	auto* row = new SaveRowButton(save, index + 1, source_, sprites_,
				      rowsHost_);
	group_->addButton(row, index);
	connect(row, &SaveRowButton::deleteRequested, this,
		[this, index]() { onDeleteClicked(index); });
	return row;
}

void SaveListScreen::onRowClicked(int index) {
	if (selecting_ || deleting_ || index < 0 ||
	    index >= static_cast<int>(saves_.size())) {
		return;
	}
	selectedSave_ = index;
	selecting_ = true;
	showState(ScreenState::Loading, "Cargando partida...");
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
	selectedSave_ = kNoSelection;
	selecting_ = false;
	showState(ScreenState::Error, messageFor(error));
}

void SaveListScreen::onDeleteClicked(int index) {
	if (loading_ || selecting_ || deleting_ || index < 0 ||
	    index >= static_cast<int>(saves_.size())) {
		return;
	}
	deleting_ = true;
	showState(ScreenState::Loading, "Eliminando partida...");
	saveService_->deleteSave(saves_[index].id);
}

void SaveListScreen::onSaveDeleted(std::uint32_t saveId) {
	if (!deleting_) {
		return;  // respuesta tardia tras onExit()
	}
	deleting_ = false;
	std::erase_if(saves_, [saveId](const SaveSummary& save) {
		return save.id == saveId;
	});
	showSaves(std::move(saves_));
}

void SaveListScreen::onDeleteFailed(ServiceError error) {
	if (!deleting_) {
		return;
	}
	deleting_ = false;
	showState(ScreenState::Error, messageFor(error));
}

QString SaveListScreen::messageFor(ServiceError error) const {
	const auto message = errorMessages_.find(error);
	if (message == errorMessages_.end()) {
		return "No se pudieron abrir las partidas.";
	}
	return message->second;
}
