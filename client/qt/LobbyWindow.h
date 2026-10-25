#pragma once

#include <QWidget>

#include <map>
#include <optional>

#include "common/model/draft/WorldSummary.h"
#include "core/SessionConfig.h"
#include "qt/screens/Screen.h"

namespace core {
class WindowConfig;
}

class QStackedWidget;
class WorldService;

// Shell del lobby: contiene las pantallas, navega entre ellas y entrega el
// resultado a Application. No tiene lógica de juego ni red.
class LobbyWindow : public QWidget {
	Q_OBJECT

public:
	explicit LobbyWindow(const core::WindowConfig& config,
			     QWidget* parent = nullptr);

	// Resultado del lobby. Vacío si el usuario cerró sin confirmar.
	std::optional<core::SessionConfig> result() const;

private:
	QStackedWidget* stack_;
	std::map<ScreenId, Screen*> screens_;
	Screen* current_;
	// Se declara como puerto: el fake solo aparece en el constructor del .cpp.
	WorldService* worldService_;
	// Mundo elegido en U2; U3b lo usa para armar el GameSelection.
	std::optional<WorldSummary> world_;
	std::optional<core::SessionConfig> selection_;

	void registerScreen(ScreenId id, Screen* screen);
	void navigateTo(ScreenId id);
	void goBack();
	void acceptSelection(core::SessionConfig session);
	void onWorldChosen(WorldSummary world);
};
