#include "PaletteView.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "Constants.h"

PaletteView::PaletteView(QWidget* parent) : QWidget(parent) {
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(editor::kPaletteMargins);

	scrollArea_ = new QScrollArea(this);
	scrollArea_->setFrameShape(QFrame::NoFrame);
	scrollArea_->setWidgetResizable(true);
	QWidget* scrollContent = new QWidget(this);
	scrollLayout_ = new QVBoxLayout(scrollContent);
	scrollLayout_->setContentsMargins(0, 0, 0, 0);
	// addPaletteGroup inserta antes de este stretch.
	scrollLayout_->addStretch();
	scrollArea_->setWidget(scrollContent);

	// Los grupos que estaran disponibles en el lienzo.
	addPaletteGroup("Arboles",
			{"Pinus", "Roble", "Palmera", "Arbol muerto"});
	addPaletteGroup("Arbustos", {"Arbusto espinoso", "Seto bajo", "Hongos"});
	addPaletteGroup("Caminos",
			{"Sendero de tierra", "Pavimento", "Adoquin"});
	addPaletteGroup("Rocas y recursos", {"Roca grande", "Roca chica", "Mina"});
	addPaletteGroup("Decoracion",
			{"Arco de piedra", "Laguna", "Ruinas"});

	layout->addWidget(scrollArea_, 1);
}

void PaletteView::addPaletteGroup(const QString& title,
				  const QStringList& items) {
	QGroupBox* group = new QGroupBox(title, scrollArea_->widget());
	QHBoxLayout* groupLayout = new QHBoxLayout(group);
	groupLayout->setContentsMargins(editor::kPaletteMargins);
	for (const QString& item : items) {
		QPushButton* button = new QPushButton(item, group);
		button->setToolTip(item);
		groupLayout->addWidget(button);
	}
	scrollLayout_->insertWidget(scrollLayout_->count() - 1, group);
}
