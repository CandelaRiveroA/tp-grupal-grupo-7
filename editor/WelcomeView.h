#pragma once

#include <string>

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "world_model/WorldRepository.h"

// Pantalla inicial
class WelcomeView : public QWidget {
	Q_OBJECT
public:
	explicit WelcomeView(const worldmodel::WorldRepository& repository,
			     QWidget* parent = nullptr);

	void refresh();

signals:
	void newWorldRequested();
	void openWorldRequested(const std::string& name);

private:
	std::string selectedName() const;

	const worldmodel::WorldRepository& repository_;
	QLabel* emptyLabel_ = nullptr;
	QListWidget* worlds_ = nullptr;
	QPushButton* openButton_ = nullptr;
};
