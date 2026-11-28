#include "MarkerSprites.h"

#include "Constants.h"

MarkerSprites::MarkerSprites()
	: defaultSprite_(editor::kDefaultMarkerSpritePath) {
	const QPixmap windows(editor::kWindowsMarkersSpritePath);
	sprites_.emplace(worldmodel::MarkerKind::Bed,
			 QPixmap(editor::kBedMarkerSpritePath));
	sprites_.emplace(worldmodel::MarkerKind::GeneralWindow,
			 windows.copy(editor::kGeneralWindowSpriteRect));
	sprites_.emplace(worldmodel::MarkerKind::CoopWindow,
			 windows.copy(editor::kCoopWindowSpriteRect));
}

const QPixmap& MarkerSprites::sprite(worldmodel::MarkerKind kind) const {
	const auto it = sprites_.find(kind);
	if (it != sprites_.end()) {
		return it->second;
	}
	return defaultSprite_;
}
