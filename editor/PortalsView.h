#pragma once

#include <QIcon>
#include <QWidget>
#include <array>

#include "world_model/Portal.h"

class QComboBox;
class QListWidget;

// Panel lateral para elegir un tipo de portal y su tamano de celdas.
class PortalsView : public QWidget {
	Q_OBJECT

public:
	explicit PortalsView(QWidget* parent = nullptr);

	void clearSelection();

signals:
	void portalPicked(worldmodel::PortalKind kind, int width, int height);

private:
	struct SizeOption {
		const char* label;
		int width;
		int height;
	};

	void populatePortals();
	SizeOption currentSizeOption() const;
	QIcon portalPreviewIcon(int width, int height) const;

	QComboBox* sizeCombo_ = nullptr;
	QListWidget* list_ = nullptr;
	std::array<SizeOption, 4> sizeOptions_{{
		{"1x1 (1 celda)", 1, 1},
		{"2x1 (horizontal)", 2, 1},
		{"1x2 (vertical)", 1, 2},
		{"2x2 (bloque)", 2, 2},
	}};
};
