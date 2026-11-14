#pragma once

#include <string>

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "world_model/MapRules.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"


class AddExtraMapDialog : public QDialog {
	Q_OBJECT
public:
	AddExtraMapDialog(const worldmodel::ScenarioLibrary& library,
			  const worldmodel::World& world,
			  QWidget* parent = nullptr);

	const worldmodel::Scenario* scenario() const;
	std::string mapId() const;

private:
	void refreshValid();

	const worldmodel::ScenarioLibrary& library_;
	const worldmodel::World& world_;
	worldmodel::MapRules rules_;
	QLineEdit* idEdit_ = nullptr;
	QComboBox* scenarioCombo_ = nullptr;
	QLabel* validityLabel_ = nullptr;
	QPushButton* acceptButton_ = nullptr;
};
