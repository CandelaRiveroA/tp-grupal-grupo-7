#pragma once

#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

// Paleta de objetos para decorar
class PaletteView : public QWidget {
	Q_OBJECT
public:
	explicit PaletteView(QWidget* parent = nullptr);

private:
	void addPaletteGroup(const QString& title,
			     const QStringList& items);

	QScrollArea* scrollArea_ = nullptr;
	QVBoxLayout* scrollLayout_ = nullptr;
};
