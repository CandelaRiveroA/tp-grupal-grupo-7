#include "LobbyWindow.h"

#include <QApplication>
#include <QShortcut>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <utility>

#include "core/WindowConfig.h"
#include "qt/screens/MainMenuScreen.h"

LobbyWindow::LobbyWindow(const core::WindowConfig& config, QWidget* parent)
	: QWidget(parent)
	, stack_(nullptr)
	, screens_()
	, selection_(std::nullopt) {
	setWindowTitle(QString::fromStdString(config.title()));
	setFixedSize(1280, 720);

	stack_ = new QStackedWidget(this);
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(stack_);

	auto* menu = new MainMenuScreen(stack_);
	registerScreen(ScreenId::MainMenu, menu);

	stack_->setCurrentWidget(menu);
	menu->onEnter();

	auto* back = new QShortcut(QKeySequence(Qt::Key_Escape), this);
	connect(back, &QShortcut::activated, this, &LobbyWindow::goBack);
}

std::optional<core::SessionConfig> LobbyWindow::result() const {
	return selection_;
}

void LobbyWindow::registerScreen(ScreenId id, Screen* screen) {
	screens_[id] = screen;
	stack_->addWidget(screen);
	connect(screen, &Screen::requestNavigate, this, &LobbyWindow::navigateTo);
}

void LobbyWindow::navigateTo(ScreenId id) {
	auto it = screens_.find(id);
	if (it == screens_.end() || it->second == stack_->currentWidget()) {
		// Destino todavía no implementado: no hay a dónde ir.
		return;
	}

	// onExit se llama al widget actual (origen), no al destino.
	stack_->currentWidget()->onExit();
	stack_->setCurrentWidget(it->second);
	it->second->onEnter();
}

void LobbyWindow::goBack() { navigateTo(ScreenId::MainMenu); }

void LobbyWindow::acceptSelection(core::SessionConfig session) {
	selection_ = std::move(session);
	close();
	QApplication::quit();
}
