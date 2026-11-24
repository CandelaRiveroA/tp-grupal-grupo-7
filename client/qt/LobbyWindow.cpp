#include "LobbyWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <utility>
#include <vector>

#include "core/WindowConfig.h"
#include "qt/screens/CharacterScreen.h"
#include "qt/screens/JoinCodeScreen.h"
#include "qt/screens/MainMenuScreen.h"
#include "qt/screens/SaveListScreen.h"
#include "qt/screens/UsernameScreen.h"
#include "qt/services/NetworkSessionService.h"
#include "qt/services/NetworkWorldService.h"
#include "qt/services/WorldService.h"
#include "qt/services/fakes/FakeSaveService.h"
#include "qt/services/CharacterSpriteComposer.h"
#include "qt_theme/GameCursor.h"

LobbyWindow::LobbyWindow(const core::WindowConfig& config,
			 const world::CharacterCatalog& characters,
			 const world::CharacterPalettes& palettes,
			 core::MusicControl* music, QWidget* parent)
	: QWidget(parent)
	, stack_(nullptr)
	, screens_()
	, current_(nullptr)
	, character_(nullptr)
	, worldService_(nullptr)
	, worldRequestPending_(false)
	, selection_(std::nullopt) {
	setWindowTitle(QString::fromStdString(config.title()));
	setFixedSize(1280, 720);
	setCursor(GameCursor().cursor());

    stack_ = new QStackedWidget(this);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stack_);

	auto* username = new UsernameScreen(stack_, music);
	registerScreen(ScreenId::Username, username);
	connect(username, &UsernameScreen::usernameConfirmed, this,
		&LobbyWindow::onUsernameConfirmed);

	auto* menu = new MainMenuScreen(stack_, music);
	registerScreen(ScreenId::MainMenu, menu);
	connect(menu, &MainMenuScreen::exitRequested, this, &QWidget::close);
	connect(menu, &MainMenuScreen::newGameRequested, this,
		&LobbyWindow::onNewGameRequested);

	worldService_ = new NetworkWorldService(this);
	connect(worldService_, &WorldService::worldsReady, this,
		[this](std::vector<WorldSummary> worlds) {
			if (!worldRequestPending_) {
				return;
			}
			worldRequestPending_ = false;
			if (worlds.empty()) {
				QMessageBox::information(this, "Mundos no disponibles",
							 "El servidor no ofrece mundos válidos.");
				return;
			}
			character_->startNewGame(worlds);
			navigateTo(ScreenId::Character);
		});
	connect(worldService_, &WorldService::worldsFailed, this,
		[this](ServiceError) {
			if (!worldRequestPending_) {
				return;
			}
			worldRequestPending_ = false;
			QMessageBox::warning(this, "Error de conexión",
					     "No se pudo obtener la lista de mundos del servidor.");
		});

	sessions_ = new NetworkSessionService(this);
	auto* sessionService = sessions_;
	auto* composer = new CharacterSpriteComposer(characters, palettes, this);

	auto* joins = new JoinCodeScreen(stack_, music);
	registerScreen(ScreenId::JoinCode, joins);
	connect(joins, &JoinCodeScreen::joinAccepted, this,
		&LobbyWindow::onJoinAccepted);

	auto* saveService = new FakeSaveService(std::nullopt, this);
	auto* saves =
		new SaveListScreen(saveService, sessionService, composer,
				   stack_);
	registerScreen(ScreenId::SaveList, saves);
	connect(saves, &SaveListScreen::saveLoaded, this,
		&LobbyWindow::onSaveLoaded);

	character_ = new CharacterScreen(sessionService, composer, stack_, music);
	registerScreen(ScreenId::Character, character_);
	connect(character_, &CharacterScreen::characterConfirmed, this,
		&LobbyWindow::onCharacterConfirmed);

    current_ = username;
    stack_->setCurrentWidget(username);
    username->onEnter();

    auto* back = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(back, &QShortcut::activated, this, &LobbyWindow::goBack);
}

std::optional<core::SessionConfig> LobbyWindow::result() { return std::move(selection_); }

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
    current_->setUsername(username_);
    stack_->setCurrentWidget(current_);
    current_->onEnter();
}

void LobbyWindow::goBack() {
    const auto username = screens_.find(ScreenId::Username);
    if (username != screens_.end() && current_ == username->second) {
        return;
    }
    sessions_->discardConnection();
	worldRequestPending_ = false;
    navigateTo(ScreenId::MainMenu);
}

void LobbyWindow::onUsernameConfirmed(std::string username) {
    username_ = std::move(username);
    navigateTo(ScreenId::MainMenu);
}

void LobbyWindow::acceptSelection(core::SessionConfig session) {
    session.connection = sessions_->takeConnection();
    selection_ = std::move(session);
    close();
    QApplication::quit();
}

void LobbyWindow::onNewGameRequested() {
	if (worldRequestPending_) {
		return;
	}
	worldRequestPending_ = true;
	worldService_->requestWorlds();
}

void LobbyWindow::onCharacterConfirmed(CharacterSelection selection) {
    core::SessionConfig session;
    session.partida = selection.gameName;
    session.personaje = selection.characterName;
    session.worldName = selection.worldName;
    acceptSelection(std::move(session));
}

void LobbyWindow::onSaveLoaded(SaveSummary save) {
    // El jugador local al retomar es el primero de la partida guardada.
    // TODO con protocolo: confirmar cómo se identifica al jugador que
    // pide la carga.
    const std::string character = save.players.empty() ? std::string() : save.players.front().name;
    core::SessionConfig session;
    session.partida = save.gameName;
    session.personaje = character;
    session.worldName = save.worldName;
    acceptSelection(std::move(session));
}

void LobbyWindow::onJoinAccepted(std::string code) {
    // La unión al server se hace en CharacterScreen, al confirmar el personaje.
    character_->startJoinGame(code);
    navigateTo(ScreenId::Character);
}
