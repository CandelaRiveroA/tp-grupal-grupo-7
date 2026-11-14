#pragma once

#include <string>

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "world_model/Map.h"

// Lienzo provisorio.
class MapPlaceholderView : public QWidget {
	Q_OBJECT
public:
	explicit MapPlaceholderView(QWidget* parent = nullptr);

	void setActiveMap(const worldmodel::Map& map);

signals:
	void changeScenarioRequested();

private:
	QLabel* mapId_ = nullptr;
	QLabel* dimensions_ = nullptr;
};
