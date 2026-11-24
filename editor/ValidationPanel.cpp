#include "ValidationPanel.h"

#include <QHBoxLayout>
#include <QPixmap>
#include <QVBoxLayout>

#include "Constants.h"
#include "NineSliceFrame.h"
#include "world_model/IssueFormatter.h"

ValidationPanel::ValidationPanel(QWidget* parent) : QWidget(parent) {
	NineSliceFrame* frame = new NineSliceFrame(":/ui/join/input_box.png",
						   editor::kWindowSlice,
						   editor::kWindowScale, this);

	QLabel* icon = new QLabel(frame);
	icon->setPixmap(QPixmap(":/editor/ui/icon_validate.png"));
	status_ = new QLabel(frame);
	status_->setObjectName("validationStatus");
	issues_ = new QListWidget(frame);
	issues_->setObjectName("issueList");
	issues_->setWordWrap(true);

	QHBoxLayout* header = new QHBoxLayout;
	header->addWidget(icon);
	header->addWidget(status_, 1);

	QVBoxLayout* layout = new QVBoxLayout(frame);
	layout->addLayout(header);
	layout->addWidget(issues_);

	QVBoxLayout* root = new QVBoxLayout(this);
	root->setContentsMargins(0, 0, 0, 0);
	root->addWidget(frame);

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
		status_->setStyleSheet("color: #2f6b1f;");
		return;
	}
	status_->setText(QString::number(issues.size()) +
			 " problemas encontrados");
	status_->setStyleSheet("color: #8b1e1e;");
	const worldmodel::IssueFormatter formatter;
	for (const worldmodel::ValidationIssue& issue : issues) {
		QListWidgetItem* item = new QListWidgetItem(
			QString::fromStdString(formatter.format(issue)));
		item->setData(Qt::UserRole,
			      QString::fromStdString(issue.mapId));
		issues_->addItem(item);
	}
}
