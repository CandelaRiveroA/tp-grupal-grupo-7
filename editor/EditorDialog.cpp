#include "EditorDialog.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "Constants.h"
#include "NineSliceFrame.h"
#include "qt_theme/GameCursor.h"

EditorDialog::EditorDialog(QWidget* parent) : QDialog(parent) {
	setObjectName("editorDialog");
	setCursor(GameCursor().cursor());
}

QWidget* EditorDialog::setupFrame(const QString& title) {
	setWindowTitle(title);
	NineSliceFrame* frame = new NineSliceFrame(":/ui/join/input_box.png",
						   editor::kWindowSlice,
						   editor::kWindowScale, this);
	QVBoxLayout* root = new QVBoxLayout(this);
	root->setContentsMargins(0, 0, 0, 0);
	root->addWidget(frame);
	return frame;
}

void EditorDialog::info(const QString& title, const QString& text) {
	QWidget* frame = setupFrame(title);
	QLabel* msg = new QLabel(text, frame);
	msg->setObjectName("editorDialogText");
	msg->setWordWrap(true);

	QPushButton* ok = new QPushButton("Aceptar", frame);
	ok->setObjectName("editorButton");
	connect(ok, &QPushButton::clicked, this, &QDialog::accept);

	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(ok);

	QVBoxLayout* layout = new QVBoxLayout(frame);
	layout->addWidget(msg);
	layout->addLayout(buttons);
	setFixedSize(sizeHint());
	exec();
}

bool EditorDialog::confirm(const QString& title, const QString& text) {
	QWidget* frame = setupFrame(title);
	QLabel* msg = new QLabel(text, frame);
	msg->setObjectName("editorDialogText");
	msg->setWordWrap(true);

	QPushButton* no = new QPushButton("No", frame);
	no->setObjectName("editorButton");
	QPushButton* yes = new QPushButton("Si", frame);
	yes->setObjectName("editorButton");
	connect(no, &QPushButton::clicked, this, &QDialog::reject);
	connect(yes, &QPushButton::clicked, this, &QDialog::accept);

	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(no);
	buttons->addWidget(yes);

	QVBoxLayout* layout = new QVBoxLayout(frame);
	layout->addWidget(msg);
	layout->addLayout(buttons);
	setFixedSize(sizeHint());
	return exec() == QDialog::Accepted;
}

std::optional<QString> EditorDialog::pickItem(const QString& title,
					      const QString& label,
					      const QStringList& items) {
	QWidget* frame = setupFrame(title);
	QLabel* prompt = new QLabel(label, frame);
	prompt->setObjectName("editorDialogText");

	QComboBox* combo = new QComboBox(frame);
	combo->setObjectName("editorCombo");
	combo->addItems(items);

	QPushButton* cancel = new QPushButton("Cancelar", frame);
	cancel->setObjectName("editorButton");
	QPushButton* accept = new QPushButton("Aceptar", frame);
	accept->setObjectName("editorButton");
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
	connect(accept, &QPushButton::clicked, this, &QDialog::accept);

	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(cancel);
	buttons->addWidget(accept);

	QVBoxLayout* layout = new QVBoxLayout(frame);
	layout->addWidget(prompt);
	layout->addWidget(combo);
	layout->addLayout(buttons);
	setFixedSize(sizeHint());
	if (exec() != QDialog::Accepted) {
		return std::nullopt;
	}
	return combo->currentText();
}
