#pragma once

#include <QWidget>

#include <map>
#include <optional>

#include "core/SessionConfig.h"
#include "qt/screens/Screen.h"

namespace core {
class WindowConfig;
}

class QStackedWidget;

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
	std::optional<core::SessionConfig> selection_;

	void registerScreen(ScreenId id, Screen* screen);
	void navigateTo(ScreenId id);
	void goBack();
	void acceptSelection(core::SessionConfig session);
};
