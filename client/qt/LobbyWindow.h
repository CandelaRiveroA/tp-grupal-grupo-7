#pragma once

#include <QWidget>

#include <map>
#include <optional>
#include <string>

#include "common/model/draft/WorldSummary.h"
#include "core/SessionConfig.h"
#include "qt/screens/Screen.h"

namespace core {
class MusicControl;
class WindowConfig;
}

namespace world {
class CharacterCatalog;
struct CharacterPalettes;
}

class CharacterScreen;
class QStackedWidget;
struct CharacterSelection;
struct SaveSummary;

// Shell del lobby: contiene las pantallas, navega entre ellas y entrega el
// resultado a Application. No tiene lógica de juego ni red.
class LobbyWindow : public QWidget {
	Q_OBJECT

public:
	LobbyWindow(const core::WindowConfig& config,
		    const world::CharacterCatalog& characters,
		    const world::CharacterPalettes& palettes,
		    core::MusicControl* music = nullptr,
		    QWidget* parent = nullptr);

	// Resultado del lobby. Vacío si el usuario cerró sin confirmar.
	std::optional<core::SessionConfig> result() const;

private:
	QStackedWidget* stack_;
	std::map<ScreenId, Screen*> screens_;
	Screen* current_;
	// Se usa después de construirla: recibe el mundo antes de navegar.
	CharacterScreen* character_;
	std::optional<core::SessionConfig> selection_;

	void registerScreen(ScreenId id, Screen* screen);
	void navigateTo(ScreenId id);
	void goBack();
	void acceptSelection(core::SessionConfig session);
	void onWorldChosen(WorldSummary world);
	void onCharacterConfirmed(CharacterSelection selection);
	void onSaveLoaded(SaveSummary save);
	void onJoinAccepted(std::string gameName);
};
