#include "LobbyWindow.h"

#include <QApplication>
#include <QShortcut>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <utility>

#include "core/WindowConfig.h"
#include "qt/screens/MainMenuScreen.h"
#include "qt/screens/WorldSelectScreen.h"
#include "qt/services/fakes/FakeWorldService.h"

LobbyWindow::LobbyWindow(const core::WindowConfig& config, QWidget* parent)
	: QWidget(parent)
	, stack_(nullptr)
	, screens_()
	, current_(nullptr)
	, world_(std::nullopt)
	, selection_(std::nullopt) {
	setWindowTitle(QString::fromStdString(config.title()));
	setFixedSize(1280, 720);

	stack_ = new QStackedWidget(this);
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(stack_);

	auto* menu = new MainMenuScreen(stack_);
	registerScreen(ScreenId::MainMenu, menu);

	auto* worldService = new FakeWorldService(std::nullopt, this);
	auto* worlds = new WorldSelectScreen(worldService, stack_);
	registerScreen(ScreenId::WorldSelect, worlds);
	connect(worlds, &WorldSelectScreen::worldChosen, this,
		&LobbyWindow::onWorldChosen);

	current_ = menu;
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
	if (it == screens_.end() || it->second == current_) {
		// Destino todavía no implementado: no hay a dónde ir.
		return;
	}

	current_->onExit();
	current_ = it->second;
	stack_->setCurrentWidget(current_);
	current_->onEnter();
}

void LobbyWindow::goBack() { navigateTo(ScreenId::MainMenu); }

void LobbyWindow::acceptSelection(core::SessionConfig session) {
	selection_ = std::move(session);
	close();
	QApplication::quit();
}

void LobbyWindow::onWorldChosen(WorldSummary world) {
	world_ = std::move(world);
	// Character todavía no está registrado: navigateTo no hace nada.
	navigateTo(ScreenId::Character);
}
