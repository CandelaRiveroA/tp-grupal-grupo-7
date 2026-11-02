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
#include "qt/theme/CharacterSprites.h"
#include "qt/util/CharacterLayer.h"
#include "qt/util/CharacterLayers.h"

namespace core {
class MusicControl;
}

class CharacterLayerSource;
class CharacterPreview;
class CycleRow;
class QFrame;
class QLabel;
class QLineEdit;
class SliderRow;
class QVBoxLayout;
class TitleButton;
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
	CharacterScreen(SessionService* session, const CharacterLayerSource* source,
			QWidget* parent = nullptr,
			core::MusicControl* music = nullptr);

	// El shell llama antes de navegar: fija el mundo y limpia el borrador.
	void startNewGame(const WorldSummary& world);

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
	CharacterLayers layers_;

	CharacterSprites sprites_;

	QFrame* content_;
	CharacterPreview* preview_;
	QLabel* worldLabel_;
	QLineEdit* characterName_;
	QLineEdit* gameName_;
	TitleButton* create_;
	TitleButton* backButton_;
	QLabel* status_;
	std::map<CharacterLayer, CycleRow*> cycleRows_;
	std::map<CharacterLayer, SliderRow*> sliderRows_;
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
	QWidget* makePreviewColumn();
	QWidget* makeFormColumn();
	QLineEdit* makeInputRow(QVBoxLayout* layout, const QString& label,
				const QString& objectName, int maxLength);
	QWidget* makeRow(CharacterLayer layer, const QString& name);
	QWidget* makeSlider(CharacterLayer layer, const QString& name,
			    const QString& objectName);
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
