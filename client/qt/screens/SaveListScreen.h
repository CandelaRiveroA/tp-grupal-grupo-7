#pragma once

#include <QString>

#include <cstdint>
#include <map>
#include <vector>

#include "common/model/draft/SaveSummary.h"
#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"
#include "qt/theme/SaveListSprites.h"
#include "qt/services/SaveService.h"
#include "qt/widgets/AsyncStateView.h"

class CharacterLayerSource;
class QButtonGroup;
class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class SaveService;
class SaveRowButton;
class SessionService;
class TitleButton;
class QWidget;

constexpr int kNoSelection = -1;

// Pantalla "Reanudar partida". Pide la lista al SaveService en onEnter() y,
// cuando el usuario elige una, carga con el SessionService; el shell arma el
// SessionConfig y cierra.
class SaveListScreen : public Screen {
	Q_OBJECT

public:
	SaveListScreen(SaveService* saves, SessionService* sessions,
		       const CharacterLayerSource* source, QWidget* parent = nullptr);

	void onEnter() override;
	void onExit() override;

signals:
	// El usuario pidió cargar una partida y el servicio la aceptó.
	void saveLoaded(SaveSummary save);

private:
	SaveService* saveService_;
	SessionService* sessionService_;
	const CharacterLayerSource* source_;

	QScrollArea* scroll_;
	QWidget* rowsHost_;
	QVBoxLayout* rowsLayout_;
	QButtonGroup* group_;
	QLabel* status_;
	QPushButton* retry_;
	QLabel* window_;
	TitleButton* scrollUp_;
	TitleButton* scrollDown_;
	TitleButton* backButton_;

	AsyncStateView states_;
	SaveListSprites sprites_;
	std::map<ServiceError, QString> errorMessages_;

	std::vector<SaveSummary> saves_;
	int selectedSave_ = kNoSelection;
	bool loading_ = false;
	bool selecting_ = false;
	bool deleting_ = false;

	void buildMessages();
	void requestSaves();
	void showSaves(std::vector<SaveSummary> saves);
	void showError(ServiceError error);
	void showState(ScreenState state, const QString& message);
	void updateNavigation();
	void scrollUp();
	void scrollDown();
	void clearRows();
	QWidget* makeRow(const SaveSummary& save, int index);
	void onRowClicked(int index);
	void onGameLoaded();
	void onSessionFailed(ServiceError error);
	void onDeleteClicked(int index);
	void onSaveDeleted(std::uint32_t saveId);
	void onDeleteFailed(ServiceError error);
	QString messageFor(ServiceError error) const;
};
