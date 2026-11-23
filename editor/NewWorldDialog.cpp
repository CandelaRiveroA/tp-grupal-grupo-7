#include "NewWorldDialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Constants.h"
#include "NineSliceFrame.h"
#include "qt_theme/GameCursor.h"

NewWorldDialog::NewWorldDialog(const worldmodel::WorldRepository& worlds,
			       QWidget* parent)
	: QDialog(parent), worlds_(worlds) {
	setObjectName("newWorldDialog");
	setWindowTitle("Nuevo mundo");
	setCursor(GameCursor().cursor());

	NineSliceFrame* frame = new NineSliceFrame(":/ui/join/input_box.png",
						   editor::kWindowSlice,
						   editor::kWindowScale, this);

	nameEdit_ = new QLineEdit(frame);
	nameEdit_->setObjectName("editorField");
	nameEdit_->setPlaceholderText("Ej: Mi Granja 1");
	validityLabel_ = new QLabel(frame);
	validityLabel_->setObjectName("validityHint");

	connect(nameEdit_, &QLineEdit::textEdited, this,
		[this] { refreshValid(); });

	QPushButton* cancelButton = new QPushButton("Cancelar", frame);
	cancelButton->setObjectName("editorButton");
	createButton_ = new QPushButton("Crear mundo", frame);
	createButton_->setObjectName("editorButton");
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(createButton_, &QPushButton::clicked, this, &QDialog::accept);

	QFormLayout* form = new QFormLayout;
	form->addRow("Nombre:", nameEdit_);
	QVBoxLayout* layout = new QVBoxLayout(frame);
	layout->addLayout(form);
	layout->addWidget(validityLabel_);
	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(cancelButton);
	buttons->addWidget(createButton_);
	layout->addLayout(buttons);

	QVBoxLayout* root = new QVBoxLayout(this);
	root->setContentsMargins(0, 0, 0, 0);
	root->addWidget(frame);

	createButton_->setEnabled(false);
	// Dialogo compacto de un campo: fija el tamano al del contenido.
	setFixedSize(sizeHint());
}

void NewWorldDialog::refreshValid() {
	QString hint;
	const std::string name = worldName();
	if (name.empty()) {
		hint = "El nombre no puede ser vacio.";
	} else if (!rules_.isValidWorldName(name)) {
		hint = "El nombre no puede contener '/' ni '.' o '..' solo.";
	} else if (worlds_.exists(name)) {
		hint = "Ya existe un mundo guardado con ese nombre.";
	}
	validityLabel_->setText(hint);
	createButton_->setEnabled(hint.isEmpty());
}

std::string NewWorldDialog::worldName() const {
	return nameEdit_->text().trimmed().toStdString();
}
