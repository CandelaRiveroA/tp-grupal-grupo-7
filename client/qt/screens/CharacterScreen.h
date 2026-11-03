#pragma once

#include <QString>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "common/model/draft/WorldSummary.h"
#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"
#include "qt/theme/CharacterSprites.h"

namespace core {
class MusicControl;
}

class CharacterAppearanceEditor;
class CharacterLayerSource;
class QFrame;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class TitleButton;
class QWidget;
class SessionService;
class WorldSelectorWidget;

// Lo que eligió el usuario. El shell lo combina con el mundo para armar el
// GameSelection que recibe SessionService.
struct CharacterSelection {
	std::string characterName;
	std::string gameName;
	CharacterAppearance appearance;
};

class CharacterScreen : public Screen {
	Q_OBJECT

public:
	CharacterScreen(SessionService* session, const CharacterLayerSource* source,
			QWidget* parent = nullptr,
			core::MusicControl* music = nullptr);

	// El shell llama antes de navegar: el selector muestra los mundos a la
	// derecha, el primero queda elegido y se limpia el borrador.
	void startNewGame(const std::vector<WorldSummary>& worlds);

	// El shell llama antes de navegar: la partida y el mundo ya los definió
	// el anfitrión; solo queda customizar el personaje. Volver lleva a la
	// pantalla del código.
	void startJoinGame(const std::string& gameName);

	void onEnter() override;
	void onExit() override;

signals:
	void characterConfirmed(CharacterSelection selection);

private:
	SessionService* session_;
	core::MusicControl* music_;
	const CharacterLayerSource* source_;

	CharacterSprites sprites_;
	CharacterAppearanceEditor* editor_;

	QFrame* content_;
	QLabel* worldLabel_;
	WorldSelectorWidget* worldSelector_;
	QLineEdit* characterName_;
	QLineEdit* gameName_;
	TitleButton* create_;
	TitleButton* backButton_;
	QLabel* status_;
	std::map<ServiceError, QString> errorMessages_;

	CharacterSelection selection_;
	std::optional<WorldSummary> world_;

	// Modo de la pantalla: nueva partida (partida + mundo a cargo del
	// jugador) o unirse (partida y mundo ya definidos por el anfitrión).
	enum class Mode { NewGame, JoinGame };

	Mode mode_ = Mode::NewGame;
	// Un createGame en vuelo: bloquea reenvío y respuestas ajenas.
	bool submitting_ = false;

	void buildMessages();
	void buildUi();
	QWidget* makeFormColumn();
	QLineEdit* makeInputRow(QVBoxLayout* layout, const QString& label,
				const QString& objectName, int maxLength);
	void updateValidation();
	void submit();
	void onCreated();
	void onJoined(const std::string& gameName, std::uint32_t worldId);
	void onSessionFailed(ServiceError error);
	void onWorldSelected(WorldSummary world);
	void resetDraft();
	void showState(ScreenState state, const QString& message);
	bool canSubmit() const;
	QString messageFor(ServiceError error) const;
};
