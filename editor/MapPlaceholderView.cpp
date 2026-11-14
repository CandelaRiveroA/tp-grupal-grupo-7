#include "MapPlaceholderView.h"

#include <QVBoxLayout>

MapPlaceholderView::MapPlaceholderView(QWidget* parent) : QWidget(parent) {
	mapId_ = new QLabel(this);
	dimensions_ = new QLabel(this);
	QPushButton* changeButton =
		new QPushButton("Cambiar escenario base...", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mapId_);
	layout->addWidget(dimensions_);
	layout->addWidget(changeButton);
	layout->addStretch();

	connect(changeButton, &QPushButton::clicked, this,
		&MapPlaceholderView::changeScenarioRequested);
}

void MapPlaceholderView::setActiveMap(const worldmodel::Map& map) {
	mapId_->setText("Mapa: " + QString::fromStdString(map.id));
	dimensions_->setText(QString::fromStdString(
		"Dimensiones: " + std::to_string(map.width) + " x " +
		std::to_string(map.height) + " celdas"));
}
