#include "AddExtraMapDialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ScenarioLabel.h"

AddExtraMapDialog::AddExtraMapDialog(const worldmodel::ScenarioLibrary& library,
				     const worldmodel::World& world,
				     QWidget* parent)
	: QDialog(parent), library_(library), world_(world) {
	setWindowTitle("Agregar mapa extra");

	idEdit_ = new QLineEdit(this);
	validityLabel_ = new QLabel(this);

	scenarioCombo_ = new QComboBox(this);
	for (const worldmodel::Scenario& scenario : library.all()) {
		scenarioCombo_->addItem(scenarioLabel(scenario),
					QString::fromStdString(scenario.id));
	}

	connect(idEdit_, &QLineEdit::textChanged, this,
		&AddExtraMapDialog::refreshValid);
	connect(scenarioCombo_, &QComboBox::currentIndexChanged, this,
		&AddExtraMapDialog::refreshValid);

	QPushButton* cancelButton = new QPushButton("Cancelar", this);
	acceptButton_ = new QPushButton("Agregar", this);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(acceptButton_, &QPushButton::clicked, this, &QDialog::accept);

	QFormLayout* form = new QFormLayout;
	form->addRow("Id del mapa:", idEdit_);
	form->addRow("Escenario:", scenarioCombo_);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addLayout(form);
	layout->addWidget(validityLabel_);
	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addWidget(cancelButton);
	buttons->addWidget(acceptButton_);
	layout->addLayout(buttons);

	// Hasta que escriban un id no hay nada que agregar.
	acceptButton_->setEnabled(false);
}

void AddExtraMapDialog::refreshValid() {
	QString hint;
	const std::string id = mapId();
	if (id.empty()) {
		hint = "El id no puede ser vacio.";
	} else if (!rules_.isValidMapId(id)) {
		hint = "El id solo admite letras minusculas, numeros, '-' y "
		       "'_'.";
	} else if (rules_.find(id) != nullptr) {
		hint = "Ese id esta reservado para un mapa obligatorio.";
	} else if (world_.maps.contains(id)) {
		hint = "Ya existe un mapa con ese id.";
	} else if (scenarioCombo_->count() == 0) {
		hint = "No hay escenarios en la biblioteca.";
	}
	validityLabel_->setText(hint);
	acceptButton_->setEnabled(hint.isEmpty());
}

const worldmodel::Scenario* AddExtraMapDialog::scenario() const {
	return library_.find(
		scenarioCombo_->currentData().toString().toStdString());
}

std::string AddExtraMapDialog::mapId() const {
	return idEdit_->text().toStdString();
}
