#pragma once

#include <QIcon>
#include <QWidget>

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
};
