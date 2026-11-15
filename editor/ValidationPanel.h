#pragma once

#include <string>
#include <vector>

#include <QLabel>
#include <QListWidget>
#include <QWidget>

#include "world_model/ValidationIssue.h"

// Problemas de validacion; un item por ValidationIssue.
class ValidationPanel : public QWidget {
	Q_OBJECT
public:
	explicit ValidationPanel(QWidget* parent = nullptr);

	void showIssues(const std::vector<worldmodel::ValidationIssue>& issues);

signals:
	void mapSelected(const std::string& mapId);

private:
	QLabel* status_ = nullptr;
	QListWidget* issues_ = nullptr;
};
