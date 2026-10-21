#include "LobbyWindow.h"

#include <QComboBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/WindowConfig.h"

namespace qt {

LobbyWindow::LobbyWindow(const core::WindowConfig& config, QWidget* parent)
	: QWidget(parent)
	, nameEdit_(nullptr)
	, gamesList_(nullptr)
	, skinBox_(nullptr)
	, joinButton_(nullptr)
	, session_()
	, confirmed_(false) {
	setWindowTitle(QString::fromStdString(config.title()));
	setFixedSize(360, 420);

	auto* layout = new QVBoxLayout(this);

	// --- Crear partida ---
	layout->addWidget(new QLabel("Crear partida", this));
	nameEdit_ = new QLineEdit(this);
	nameEdit_->setPlaceholderText("Nombre de la partida");
	layout->addWidget(nameEdit_);

	auto* createButton = new QPushButton("Crear", this);
	layout->addWidget(createButton);

	// --- Unirse a partida existente ---
	layout->addWidget(new QLabel("Partidas existentes (doble clic para unirse)", this));
	gamesList_ = new QListWidget(this);
	layout->addWidget(gamesList_);

	joinButton_ = new QPushButton("Unirme", this);
	joinButton_->setEnabled(false);
	layout->addWidget(joinButton_);

	// --- Personaje ---
	auto* skinRow = new QHBoxLayout();
	skinRow->addWidget(new QLabel("Personaje", this));
	skinBox_ = new QComboBox(this);
	skinBox_->addItems({ "Granjero", "Minero" });
	skinRow->addWidget(skinBox_, 1);
	layout->addLayout(skinRow);

	connect(createButton, &QPushButton::clicked, this, &LobbyWindow::createGame);
	connect(joinButton_, &QPushButton::clicked, this, &LobbyWindow::joinSelectedGame);
	connect(gamesList_, &QListWidget::itemDoubleClicked, this,
		[this](QListWidgetItem*) { joinSelectedGame(); });
	connect(gamesList_, &QListWidget::itemSelectionChanged, this, [this]() {
		joinButton_->setEnabled(gamesList_->currentItem() != nullptr);
	});
	connect(nameEdit_, &QLineEdit::returnPressed, this, &LobbyWindow::createGame);
}

void LobbyWindow::createGame() {
	session_.partida = nameEdit_->text().trimmed().toStdString();
	if (session_.partida.empty()) {
		return;
	}

	// TODO(protocolo): acá se le pediría al server crear la partida
	// y validar que el usuario no esté tomado.
	confirm();
}

void LobbyWindow::joinSelectedGame() {
	auto* item = gamesList_->currentItem();
	if (item == nullptr) {
		return;
	}
	session_.partida = item->text().toStdString();

	// TODO(protocolo): acá se valida con el server que la partida
	// exista y que el usuario no esté tomado.
	confirm();
}

void LobbyWindow::confirm() {
	session_.personaje = skinBox_->currentText().toStdString();
	confirmed_ = true;
	close();
	QApplication::quit();
}

void LobbyWindow::cancel() {
	confirmed_ = false;
	close();
	QApplication::quit();
}

bool LobbyWindow::confirmed() const { return confirmed_; }

const core::SessionConfig& LobbyWindow::session() const { return session_; }

}
