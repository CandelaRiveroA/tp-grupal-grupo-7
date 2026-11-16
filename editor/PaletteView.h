#pragma once

#include <QWidget>
#include <string>

#include "world_model/PrefabCatalog.h"

class QComboBox;
class QListWidget;
class TilesetCache;

// Panel de prefabs con selector de categoria y previsualizacion completa.
class PaletteView : public QWidget {
	Q_OBJECT
public:
	PaletteView(const worldmodel::PrefabCatalog& catalog,
		    const TilesetCache& tilesets, QWidget* parent = nullptr);

	void clearSelection();

signals:
	void prefabSelected(const worldmodel::PrefabDef* prefab);

private:
	void populateCategory(const std::string& category);
	QPixmap centeredPreview(const QPixmap& sprite) const;

	const worldmodel::PrefabCatalog& catalog_;
	const TilesetCache& tilesets_;
	QComboBox* categoryCombo_ = nullptr;
	QListWidget* list_ = nullptr;
};
