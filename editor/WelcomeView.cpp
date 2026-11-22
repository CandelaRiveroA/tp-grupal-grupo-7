#include "WelcomeView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

WelcomeView::WelcomeView(const worldmodel::WorldRepository& repository,
			 QWidget* parent)
	: QWidget(parent), repository_(repository) {
	QLabel* heading = new QLabel("<h1>Editor de mundos</h1>", this);

	worlds_ = new QListWidget(this);
	openButton_ = new QPushButton("Abrir mundo seleccionado", this);
	openButton_->setEnabled(false);
	QPushButton* newButton = new QPushButton("Nuevo mundo...", this);

	emptyLabel_ = new QLabel("Todavia no hay mundos guardados", this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addStretch();
	layout->addWidget(heading);
	heading->setAlignment(Qt::AlignHCenter);
	emptyLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	layout->addWidget(emptyLabel_);
	layout->addWidget(worlds_);
	layout->addStretch();
	QHBoxLayout* buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(openButton_);
	buttons->addWidget(newButton);
	buttons->addStretch();
	layout->addLayout(buttons);

	connect(worlds_, &QListWidget::itemSelectionChanged, this, [this] {
		openButton_->setEnabled(!selectedName().empty());
	});
	const auto openSelected = [this] {
		const std::string name = selectedName();
		if (!name.empty()) {
			emit openWorldRequested(name);
		}
	};
	connect(worlds_, &QListWidget::itemDoubleClicked, this, openSelected);
	connect(openButton_, &QPushButton::clicked, this, openSelected);
	connect(newButton, &QPushButton::clicked, this,
		&WelcomeView::newWorldRequested);
}

void WelcomeView::refresh() {
	worlds_->clear();
	const std::vector<worldmodel::WorldName> names =
		repository_.listWorlds();
	emptyLabel_->setVisible(names.empty());
	for (const worldmodel::WorldName& name : names) {
		QListWidgetItem* item = new QListWidgetItem(
			QString::fromStdString(name));
		item->setData(Qt::UserRole, QString::fromStdString(name));
		worlds_->addItem(item);
	}
}

std::string WelcomeView::selectedName() const {
	const QListWidgetItem* item = worlds_->currentItem();
	if (item == nullptr) {
		return {};
	}
	return item->data(Qt::UserRole).toString().toStdString();
}
