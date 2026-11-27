#pragma once

#include <QIcon>
#include <QPixmap>
#include <QWidget>
#include <vector>

#include "PortalPrefabs.h"
#include "world_model/Portal.h"

class QComboBox;
class QListWidget;
class TilesetCache;

namespace worldmodel {
class PrefabCatalog;
}

// Panel lateral para elegir un tipo de portal y su tamano de celdas.
class PortalsView : public QWidget {
	Q_OBJECT

public:
	PortalsView(const worldmodel::PrefabCatalog& catalog,
		    const TilesetCache& tilesets, QWidget* parent = nullptr);

	void clearSelection();

signals:
	void portalPicked(worldmodel::PortalKind kind, int width, int height);

private:
	struct SizeOption {
		const char* label;
		int width;
		int height;
	};

	struct PortalEntry {
		worldmodel::PortalKind kind;
		const char* label;
	};

	const worldmodel::PrefabCatalog& catalog_;
	const TilesetCache& tilesets_;
	PortalPrefabs portalPrefabs_;
	QComboBox* sizeCombo_ = nullptr;
	QListWidget* list_ = nullptr;
	QPixmap rune_;
	std::vector<SizeOption> sizeOptions_;
	std::vector<PortalEntry> portalEntries_;

	void populatePortals();
	SizeOption currentSizeOption() const;
	QIcon portalPreviewIcon(int width, int height) const;
};
