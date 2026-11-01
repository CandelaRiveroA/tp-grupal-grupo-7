#pragma once

#include <QWidget>
#include <map>
#include <optional>
#include <string>

#include "common/model/draft/WorldSummary.h"
#include "core/SessionConfig.h"
#include "qt/screens/Screen.h"

namespace core {
class WindowConfig;
}

class CharacterScreen;
class NetworkSessionService;
class QStackedWidget;
struct CharacterSelection;
struct SaveSummary;

// Shell del lobby: contiene las pantallas, navega entre ellas y entrega el
// resultado a Application. No tiene lógica de juego ni red.
class LobbyWindow: public QWidget {
    Q_OBJECT

public:
    explicit LobbyWindow(const core::WindowConfig& config, QWidget* parent = nullptr);

    // Resultado del lobby. Vacío si el usuario cerró sin confirmar.
    std::optional<core::SessionConfig> result();

private:
    QStackedWidget* stack_;
    std::map<ScreenId, Screen*> screens_;
    Screen* current_;
    // Se usa después de construirla: recibe el mundo antes de navegar.
    CharacterScreen* character_;
    NetworkSessionService* sessions_;
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
