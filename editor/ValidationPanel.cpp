#include "ValidationPanel.h"

#include <QVBoxLayout>

#include "world_model/IssueFormatter.h"

ValidationPanel::ValidationPanel(QWidget* parent) : QWidget(parent) {
	status_ = new QLabel(this);
	issues_ = new QListWidget(this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(status_);
	layout->addWidget(issues_);

	connect(issues_, &QListWidget::itemClicked, this,
		[this](QListWidgetItem* item) {
			emit mapSelected(item->data(Qt::UserRole)
						 .toString()
						 .toStdString());
		});
}

void ValidationPanel::showIssues(
	const std::vector<worldmodel::ValidationIssue>& issues) {
	issues_->clear();
	if (issues.empty()) {
		status_->setText("Mundo valido (0 errores)");
		status_->setStyleSheet("color: #27ae60;");
		return;
	}
	status_->setText(QString::number(issues.size()) +
			 " problemas encontrados");
	status_->setStyleSheet("color: #c0392b;");
	const worldmodel::IssueFormatter formatter;
	for (const worldmodel::ValidationIssue& issue : issues) {
		QListWidgetItem* item = new QListWidgetItem(
			QString::fromStdString(formatter.format(issue)));
		item->setData(Qt::UserRole,
			      QString::fromStdString(issue.mapId));
		issues_->addItem(item);
	}
}
