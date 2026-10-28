#pragma once

#include <QString>

#include <cstdint>
#include <vector>

#include "common/model/draft/WorldSummary.h"
#include "qt/screens/Screen.h"
#include "qt/services/ServiceError.h"
#include "qt/widgets/AsyncStateView.h"

class QButtonGroup;
class QGridLayout;
class QLabel;
class QPushButton;
class QScrollArea;
class WorldService;

// Pantalla de selección de mundo. Pide la lista en onEnter() y emite el mundo
// elegido; el shell decide a dónde ir.
class WorldSelectScreen : public Screen {
	Q_OBJECT

public:
	explicit WorldSelectScreen(WorldService* service,
				   QWidget* parent = nullptr);

	void onEnter() override;

signals:
	// El usuario confirmó un mundo. La pantalla no navega sola.
	void worldChosen(WorldSummary world);

private:
	WorldService* service_;
	QScrollArea* scroll_;
	QWidget* cardsHost_;
	QGridLayout* cardsLayout_;
	QButtonGroup* group_;
	QLabel* status_;
	QPushButton* retry_;
	QPushButton* continue_;
	AsyncStateView states_;

	std::vector<WorldSummary> worlds_;
	std::uint32_t selectedWorldId_ = 0;
	bool loading_ = false;
	bool loaded_ = false;

	void requestWorlds();
	void showWorlds(std::vector<WorldSummary> worlds);
	void showError(ServiceError error);
	void showState(ScreenState state, const QString& message);
	void clearCards();
	QPushButton* makeCard(const WorldSummary& world, int index);
	void onCardClicked(int index);
	void onContinue();

	QString messageFor(ServiceError error) const;
};
