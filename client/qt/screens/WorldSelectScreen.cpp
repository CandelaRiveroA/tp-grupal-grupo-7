#include "WorldSelectScreen.h"

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>
#include <utility>

#include "qt/services/WorldService.h"

WorldSelectScreen::WorldSelectScreen(WorldService* service, QWidget* parent)
	: Screen(parent)
	, service_(service)
	, scroll_(nullptr)
	, cardsHost_(nullptr)
	, cardsLayout_(nullptr)
	, group_(nullptr)
	, status_(nullptr)
	, retry_(nullptr)
	, continue_(nullptr) {
	setObjectName("worldSelect");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(64, 48, 64, 48);
	layout->setSpacing(16);

	auto* title = new QLabel("Elegí un mundo", this);
	title->setObjectName("screenTitle");
	layout->addWidget(title);

	scroll_ = new QScrollArea(this);
	scroll_->setWidgetResizable(true);
	scroll_->setFrameShape(QFrame::NoFrame);
	scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_->setVisible(false);
	layout->addWidget(scroll_, 1);

	cardsHost_ = new QWidget(scroll_);
	cardsLayout_ = new QGridLayout(cardsHost_);
	cardsLayout_->setContentsMargins(0, 0, 0, 0);
	cardsLayout_->setSpacing(24);
	cardsLayout_->setColumnStretch(0, 1);
	cardsLayout_->setColumnStretch(1, 1);
	scroll_->setWidget(cardsHost_);

	group_ = new QButtonGroup(this);
	group_->setExclusive(true);
	connect(group_, &QButtonGroup::idClicked, this,
		&WorldSelectScreen::onCardClicked);

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
		&WorldSelectScreen::requestWorlds);
	layout->addWidget(retry_, 0, Qt::AlignHCenter);

	auto* buttons = new QHBoxLayout();
	buttons->addStretch();

	auto* back = new QPushButton("Volver", this);
	back->setObjectName("backButton");
	connect(back, &QPushButton::clicked, this, [this]() {
		emit requestNavigate(ScreenId::MainMenu);
	});
	buttons->addWidget(back);

	continue_ = new QPushButton("Continuar", this);
	continue_->setObjectName("continueButton");
	continue_->setEnabled(false);
	connect(continue_, &QPushButton::clicked, this,
		&WorldSelectScreen::onContinue);
	buttons->addWidget(continue_);

	layout->addLayout(buttons);

	connect(service_, &WorldService::worldsReady, this,
		&WorldSelectScreen::showWorlds);
	connect(service_, &WorldService::worldsFailed, this,
		&WorldSelectScreen::showError);
}

void WorldSelectScreen::onEnter() {
	if (!loaded_ && !loading_) {
		requestWorlds();
	}
}

void WorldSelectScreen::requestWorlds() {
	if (loading_) {
		return;
	}
	loading_ = true;
	showState(ScreenState::Loading, "Cargando mundos…");
	service_->requestWorlds();
}

void WorldSelectScreen::showWorlds(std::vector<WorldSummary> worlds) {
	loading_ = false;
	loaded_ = true;
	worlds_ = std::move(worlds);
	clearCards();

	const int columns = 2;
	bool selectionFound = false;

	for (int i = 0; i < static_cast<int>(worlds_.size()); ++i) {
		auto* card = makeCard(worlds_[i], i);
		cardsLayout_->addWidget(card, i / columns, i % columns,
					Qt::AlignTop);
		if (worlds_[i].id == selectedWorldId_) {
			card->setChecked(true);
			selectionFound = true;
		}
	}

	if (!selectionFound) {
		selectedWorldId_ = 0;
	}

	if (worlds_.empty()) {
		showState(ScreenState::Empty, "No hay mundos disponibles.");
		return;
	}

	showState(ScreenState::Ready, QString());
}

void WorldSelectScreen::showError(ServiceError error) {
	loading_ = false;
	loaded_ = false;
	showState(ScreenState::Error, messageFor(error));
}

void WorldSelectScreen::showState(ScreenState state, const QString& message) {
	const bool ready = state == ScreenState::Ready;

	scroll_->setVisible(ready);
	status_->setVisible(!ready);
	if (!ready) {
		status_->setText(message);
	}
	retry_->setVisible(state == ScreenState::Error);
	continue_->setEnabled(ready && selectedWorldId_ != 0);
}

void WorldSelectScreen::clearCards() {
	while (QLayoutItem* item = cardsLayout_->takeAt(0)) {
		delete item->widget();
		delete item;
	}
}

QPushButton* WorldSelectScreen::makeCard(const WorldSummary& world,
					 int index) {
	const QSize cardSize(420, 260);
	const int thumbHeight = 120;

	auto* card = new QPushButton(cardsHost_);
	card->setObjectName("worldCard");
	card->setCheckable(true);
	card->setMinimumSize(cardSize);
	card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	card->setCursor(Qt::PointingHandCursor);

	auto* layout = new QVBoxLayout(card);
	layout->setContentsMargins(16, 16, 16, 16);
	layout->setSpacing(12);

	auto* thumb = new QFrame(card);
	thumb->setObjectName("worldThumb");
	thumb->setFixedHeight(thumbHeight);
	thumb->setAttribute(Qt::WA_TransparentForMouseEvents);
	layout->addWidget(thumb);

	auto* name = new QLabel(QString::fromStdString(world.name), card);
	name->setObjectName("worldName");
	name->setAttribute(Qt::WA_TransparentForMouseEvents);
	layout->addWidget(name);

	if (!world.description.empty()) {
		auto* description = new QLabel(
			QString::fromStdString(world.description), card);
		description->setObjectName("worldDescription");
		description->setWordWrap(true);
		description->setAttribute(Qt::WA_TransparentForMouseEvents);
		layout->addWidget(description);
	}

	layout->addStretch();
	group_->addButton(card, index);
	return card;
}

void WorldSelectScreen::onCardClicked(int index) {
	if (index < 0 || index >= static_cast<int>(worlds_.size())) {
		return;
	}
	selectedWorldId_ = worlds_[index].id;
	continue_->setEnabled(true);
}

void WorldSelectScreen::onContinue() {
	const auto it = std::find_if(
		worlds_.begin(), worlds_.end(), [this](const WorldSummary& world) {
			return world.id == selectedWorldId_;
		});
	if (it == worlds_.end()) {
		return;
	}
	emit worldChosen(*it);
}

QString WorldSelectScreen::messageFor(ServiceError error) const {
	switch (error) {
	case ServiceError::Network:
		return "No se pudo conectar con el servidor.";
	default:
		return "No se pudieron cargar los mundos.";
	}
}
