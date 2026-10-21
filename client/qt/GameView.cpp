#include "GameView.h"

#include <QPaintEvent>
#include <QPainter>

#include "core/WindowConfig.h"

namespace qt {

GameView::GameView(const core::WindowConfig& config, QWidget* parent)
	: QWidget(parent)
	, frame_(config.width(), config.height(), QImage::Format_ARGB32) {

	frame_.fill(Qt::black);
}

QImage& GameView::frameBuffer() { return frame_; }

void GameView::present() { update(); }

void GameView::paintEvent(QPaintEvent* ) {
	QPainter painter(this);
	painter.drawImage(rect(), frame_);
}

}
