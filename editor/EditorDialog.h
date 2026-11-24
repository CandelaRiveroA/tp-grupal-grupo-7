#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>
#include <optional>

// Dialogos modales con marco de madera (avisos, confirmaciones y seleccion).
class EditorDialog : public QDialog {
public:
	explicit EditorDialog(QWidget* parent = nullptr);

	void info(const QString& title, const QString& text);
	bool confirm(const QString& title, const QString& text);
	std::optional<QString> pickItem(const QString& title,
					const QString& label,
					const QStringList& items);

private:
	QWidget* setupFrame(const QString& title);
};
