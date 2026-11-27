#pragma once

#include <QIcon>
#include <QListWidget>
#include <QString>
#include <QVariant>
#include <vector>

class QWidget;

// Lista en grilla IconMode con celdas uniformes ajustadas al texto completo sin recorte.
class PaletteListWidget : public QListWidget {
public:
	struct Card {
		QIcon icon;
		QString label;
		QVariant userData;
	};

	explicit PaletteListWidget(const QString& objectName,
				   QWidget* parent = nullptr);

	void setCards(const std::vector<Card>& cards);
};
