#include "WelcomeView.h"

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimerEvent>
#include <QVBoxLayout>

#include "Constants.h"
#include "NineSliceFrame.h"
#include "qt_theme/CloudRects.h"
#include "qt_theme/TitleButton.h"
#include "qt_theme/TitleSpriteRects.h"

WelcomeView::WelcomeView(const worldmodel::WorldRepository& repository,
			 QWidget* parent)
	: QWidget(parent), repository_(repository) {
	QLabel* heading = new QLabel(this);
	heading->setPixmap(QPixmap(":/editor/ui/titulo_editor.png"));
	heading->setAlignment(Qt::AlignHCenter);

	NineSliceFrame* panel = new NineSliceFrame(
		":/ui/join/input_box.png", editor::kWindowSlice,
		editor::kWindowScale, this);
	panel->setMinimumWidth(editor::kWelcomePanelMinWidth);
	panel->setMaximumWidth(editor::kWelcomePanelMaxWidth);
	panel->setMaximumHeight(editor::kWelcomePanelMaxHeight);

	emptyLabel_ = new QLabel("Todavia no hay mundos guardados", panel);
	emptyLabel_->setObjectName("welcomeEmpty");
	emptyLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	worlds_ = new QListWidget(panel);
	worlds_->setObjectName("worldList");

	QVBoxLayout* panelLayout = new QVBoxLayout(panel);
	panelLayout->setContentsMargins(editor::kWelcomePanelPadding);
	panelLayout->addWidget(emptyLabel_);
	panelLayout->addWidget(worlds_, 1);

	openButton_ = new TitleButton(
		QPixmap(":/editor/ui/boton_abrir_mundo.png"),
		QPixmap(":/editor/ui/boton_abrir_mundo_hover.png"),
		"Abrir mundo seleccionado", this);
	openButton_->setEnabled(false);
	TitleButton* newButton = new TitleButton(
		QPixmap(":/editor/ui/boton_nuevo_mundo.png"),
		QPixmap(":/editor/ui/boton_nuevo_mundo_hover.png"),
		"Nuevo mundo...", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kWelcomeMargins);
	layout->setSpacing(editor::kWelcomeSpacing);
	layout->addStretch();
	layout->addWidget(heading, 0, Qt::AlignHCenter);
	layout->addWidget(panel, 1, Qt::AlignHCenter);
	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->setSpacing(TITLE_BUTTON_SPACING);
	buttons->addStretch();
	buttons->addWidget(openButton_);
	buttons->addWidget(newButton);
	buttons->addStretch();
	layout->addLayout(buttons);
	layout->addStretch();

	connect(worlds_, &QListWidget::itemSelectionChanged, this, [this] {
		openButton_->setEnabled(!selectedName().empty());
	});
	const auto openSelected = [this] {
		const std::string name = selectedName();
		if (!name.empty()) {
			emit openWorldRequested(name);
		}
	};
	connect(worlds_, &QListWidget::itemDoubleClicked, this, openSelected);
	connect(openButton_, &QAbstractButton::clicked, this, openSelected);
	connect(newButton, &QAbstractButton::clicked, this,
		&WelcomeView::newWorldRequested);
}

void WelcomeView::refresh() {
	worlds_->clear();
	const std::vector<worldmodel::WorldName> names =
		repository_.listWorlds();
	emptyLabel_->setVisible(names.empty());
	worlds_->setVisible(!names.empty());
	for (const worldmodel::WorldName& name : names) {
		QListWidgetItem* item = new QListWidgetItem(
			QString::fromStdString(name));
		item->setData(Qt::UserRole, QString::fromStdString(name));
		worlds_->addItem(item);
	}
}

void WelcomeView::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	backdrop_.paint(painter);
	QWidget::paintEvent(event);
}

void WelcomeView::resizeEvent(QResizeEvent* event) {
	backdrop_.resize(event->size());
	QWidget::resizeEvent(event);
}

void WelcomeView::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	if (!cloudTimer_.isActive()) {
		cloudTimer_.start(CLOUD_ANIMATION_TICK_MS, this);
	}
}

void WelcomeView::hideEvent(QHideEvent* event) {
	QWidget::hideEvent(event);
	if (cloudTimer_.isActive()) {
		cloudTimer_.stop();
	}
}

void WelcomeView::timerEvent(QTimerEvent* event) {
	if (event->timerId() == cloudTimer_.timerId()) {
		backdrop_.step(float(CLOUD_ANIMATION_TICK_MS) /
			       editor::kMsPerSecond);
		update();
		return;
	}
	QWidget::timerEvent(event);
}

std::string WelcomeView::selectedName() const {
	const QListWidgetItem* item = worlds_->currentItem();
	if (item == nullptr) {
		return {};
	}
	return item->data(Qt::UserRole).toString().toStdString();
}
