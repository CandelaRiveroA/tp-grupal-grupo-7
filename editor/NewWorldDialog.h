#pragma once

#include <string>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "world_model/MapRules.h"
#include "world_model/WorldRepository.h"

// Dialogo para crear un mundo nuevo
class NewWorldDialog : public QDialog {
	Q_OBJECT
public:
	NewWorldDialog(const worldmodel::WorldRepository& worlds,
		       QWidget* parent = nullptr);

	std::string worldName() const;

private:
	void refreshValid();

	const worldmodel::WorldRepository& worlds_;
	worldmodel::MapRules rules_;
	QLineEdit* nameEdit_ = nullptr;
	QLabel* validityLabel_ = nullptr;
	QPushButton* createButton_ = nullptr;
};
