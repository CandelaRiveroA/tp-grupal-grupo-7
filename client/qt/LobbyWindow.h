#pragma once

#include <QWidget>

#include "core/SessionConfig.h"

class QLineEdit;
class QListWidget;
class QComboBox;
class QPushButton;

namespace core {
class WindowConfig;
}

namespace qt {

// Lobby mínimo (Qt): crear partida o unirse a una existente y elegir
// personaje. Al confirmar, devuelve el SessionConfig con el que arranca
// el "main SDL". No tiene lógica de juego ni red.
class LobbyWindow : public QWidget {
	Q_OBJECT

public:
	explicit LobbyWindow(const core::WindowConfig& config,
			    QWidget* parent = nullptr);

	// Cierra el lobby y deja la sesión lista para delegar en SDL.
	void confirm();
	void cancel();

	bool confirmed() const;
	const core::SessionConfig& session() const;

private:
	void createGame();
	void joinSelectedGame();

	QLineEdit* nameEdit_;
	QListWidget* gamesList_;
	QComboBox* skinBox_;
	QPushButton* joinButton_;

	core::SessionConfig session_;
	bool confirmed_;
};

}
