#include "Screen.h"

#include <QPainter>
#include <QTimerEvent>

#include <utility>

#include "qt/theme/CloudRects.h"

Screen::Screen(QWidget* parent) : QWidget(parent) {}

void Screen::onEnter() {
	if (!cloudTimer_.isActive()) {
		cloudTimer_.start(CLOUD_ANIMATION_TICK_MS, this);
	}
}

void Screen::onExit() {
	if (cloudTimer_.isActive()) {
		cloudTimer_.stop();
	}
}

void Screen::setUsername(std::string username) {
	username_ = std::move(username);
}

const std::string& Screen::username() const { return username_; }

void Screen::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	backdrop_.paint(painter);
	QWidget::paintEvent(event);
}

void Screen::timerEvent(QTimerEvent* event) {
	if (event->timerId() == cloudTimer_.timerId()) {
		backdrop_.step(float(CLOUD_ANIMATION_TICK_MS) / 1000.0f);
		update();
		return;
	}
	QWidget::timerEvent(event);
}