#include "MainWindow.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLayout>
#include <QString>

#include "core/InputManager.h"
#include "core/WindowConfig.h"

namespace qt {

MainWindow::MainWindow(const core::WindowConfig& config,
		       core::InputManager& input,
		       QWidget* parent)
	: QWidget(parent)
	, input_(input) {
	setWindowTitle(QString::fromStdString(config.title()));
	setFixedSize(config.width(), config.height());

	setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::setContent(QWidget* widget) {
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(widget);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {

	if (!event->isAutoRepeat()) {
		input_.setKey(static_cast<Qt::Key>(event->key()), true);
	}

}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
	if (!event->isAutoRepeat()) {
		input_.setKey(static_cast<Qt::Key>(event->key()), false);
	}
}

void MainWindow::focusOutEvent(QFocusEvent* event) {

	input_.clearAll();
	QWidget::focusOutEvent(event);
}

}
