#pragma once

#include <QString>

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "common/model/draft/CharacterAppearance.h"
#include "common/model/draft/WorldSummary.h"
#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"
#include "qt/util/CharacterLayer.h"
#include "qt/util/CharacterLayers.h"

class AssetCatalog;
class CharacterPreview;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QWidget;
class SessionService;

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
	CharacterScreen(SessionService* session, const AssetCatalog* catalog,
			QWidget* parent = nullptr);

	// El shell llama antes de navegar: fija el mundo y limpia el borrador.
	void startNewGame(const WorldSummary& world);

	// El shell llama antes de navegar: la partida y el mundo ya los definió
	// el anfitrión; solo queda customizar el personaje. Volver lleva a la
	// pantalla del código.
	void startJoinGame(const std::string& gameName);

	void onEnter() override;

signals:
	void characterConfirmed(CharacterSelection selection);

private:
	SessionService* session_;
	const AssetCatalog* catalog_;
	CharacterLayers layers_;

	QFrame* content_;
	CharacterPreview* preview_;
	QLabel* worldLabel_;
	QLineEdit* characterName_;
	QLineEdit* gameName_;
	QPushButton* create_;
	QPushButton* backButton_;
	QLabel* status_;
	std::map<CharacterLayer, QLabel*> values_;
	std::map<CharacterLayer, QSlider*> sliders_;
	std::map<ServiceError, QString> errorMessages_;

	CharacterSelection selection_;
	std::optional<WorldSummary> world_;

	// Modo de la pantalla: nueva partida (partida + mundo a cargo del
	// jugador) o unirse (partida y mundo ya definidos por el anfitrión).
	enum class Mode { NewGame, JoinGame };

	Mode mode_ = Mode::NewGame;
	// Un createGame en vuelo: bloquea reenvío y respuestas ajenas.
	bool submitting_ = false;

	static constexpr int kWindowMaxWidth = 960;

	void buildMessages();
	void buildUi();
	QWidget* makePreviewColumn();
	QWidget* makeFormColumn();
	QWidget* makeRow(CharacterLayer layer, const QString& name);
	QWidget* makeSlider(CharacterLayer layer, const QString& name);
	void cycleLayer(CharacterLayer layer, int direction);
	void setLayer(CharacterLayer layer, std::uint8_t index);
	void applyDraft();
	void updateValidation();
	void submit();
	void onCreated();
	void onSessionFailed(ServiceError error);
	void resetDraft();
	void showState(ScreenState state, const QString& message);
	bool canSubmit() const;
	QString messageFor(ServiceError error) const;
};
