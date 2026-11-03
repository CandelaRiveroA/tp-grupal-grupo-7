#include "LobbyWindow.h"

#include <QApplication>
#include <QShortcut>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <utility>

#include "core/WindowConfig.h"
#include "qt/screens/CharacterScreen.h"
#include "qt/screens/JoinCodeScreen.h"
#include "qt/screens/MainMenuScreen.h"
#include "qt/screens/SaveListScreen.h"
#include "qt/screens/UsernameScreen.h"
#include "qt/services/NetworkSessionService.h"
#include "qt/services/fakes/FakeSaveService.h"
#include "qt/services/fakes/FakeWorldService.h"
#include "qt/services/CharacterSpriteComposer.h"
#include "qt/theme/GameCursor.h"

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

    worldService_ = new FakeWorldService(std::nullopt, this);

	sessions_ = new NetworkSessionService(this);
	auto* sessionService = sessions_;
	auto* composer = new CharacterSpriteComposer(characters, palettes, this);

	auto* joins = new JoinCodeScreen(sessionService, stack_, music);
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
    character_->startNewGame(worldService_->worlds());
    navigateTo(ScreenId::Character);
}

void LobbyWindow::onCharacterConfirmed(CharacterSelection selection) {
    core::SessionConfig session;
    session.partida = selection.gameName;
    session.personaje = selection.characterName;
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
    acceptSelection(std::move(session));
}

void LobbyWindow::onJoinAccepted(std::string gameName) {
    // Solo queda customizar el personaje: la partida y el mundo ya los
    // definió el anfitrión. El worldId del server no nos alcanza acá.
    // TODO con protocolo: cómo viaja el personaje del joiner al server.
    character_->startJoinGame(gameName);
    navigateTo(ScreenId::Character);
}
