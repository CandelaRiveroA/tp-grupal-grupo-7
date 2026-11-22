#include "world/OverlayTomlReader.h"

#include <array>
#include <string>

#include "world/ConfigError.h"
#include "world/OverlayKeys.h"
#include "world/SceneKeys.h"
#include "world/SceneTomlReader.h"
#include "world_model/TileCodec.h"

namespace world {

OverlayTomlReader::OverlayTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

WorldOverlay OverlayTomlReader::read() const {
	WorldOverlay overlay;
	readScale(overlay);

	// Las hojas propias del cliente (opcionales) las lee el lector de
	// escenas: misma sintaxis que scenes.toml.
	SceneDescription sheets;
	SceneTomlReader(doc_).readExtraTileSets(sheets);
	for (const auto& [name, def] : sheets.tilesets) {
		overlay.tilesets.emplace(name, def);
	}

	readDecorations(overlay);
	readMarkerNpcs(overlay);
	readMapElements(overlay);
	return overlay;
}

void OverlayTomlReader::readScale(WorldOverlay& overlay) const {
	const std::string path =
		std::string(kKeyOverlayWorld) + "." + kKeyOverlayScale;
	if (!doc_.hasKey(path)) {
		return;
	}
	overlay.scale = doc_.getInt(path);
	if (overlay.scale <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}
}

void OverlayTomlReader::readDecorations(WorldOverlay& overlay) const {
	if (!doc_.hasKey(kKeyOverlayDecorations)) {
		return;
	}
	for (const std::string& key :
	     doc_.subtableKeys(kKeyOverlayDecorations)) {
		const std::string base =
			std::string(kKeyOverlayDecorations) + "." + key;
		DecorationOverlay decoration;
		const std::string nightPath =
			base + "." + kKeyOverlayNightTile;
		if (doc_.hasKey(nightPath)) {
			try {
				const std::optional<worldmodel::TileRef> tile =
					worldmodel::TileCodec::decode(
						doc_.getString(nightPath),
						nightPath);
				if (!tile.has_value()) {
					throw ConfigError(
						ConfigErrorKind::InvalidValue,
						nightPath);
				}
				decoration.night = NightVariant{
					tile->tileset, {tile->column, tile->row}};
			} catch (const worldmodel::FormatError& error) {
				throw ConfigError(error);
			}
		}
		readEnter(base, decoration);
		overlay.decorations.emplace(key, decoration);
	}
}

void OverlayTomlReader::readEnter(const std::string& base,
				  DecorationOverlay& decoration) const {
	const std::string enterPath = base + "." + kKeyEnter;
	if (!doc_.hasKey(enterPath)) {
		return;
	}
	const std::array<int, 2> rows =
		doc_.getIntArray<2>(enterPath + "." + kKeyEnterRows);
	if (rows[0] < 0 || rows[1] < rows[0]) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  enterPath + "." + kKeyEnterRows);
	}
	const Direction facing = SceneTomlReader(doc_).parseFacing(
		doc_.getString(enterPath + "." + kKeyEnterFacing), enterPath);
	const std::string markerPath =
		enterPath + "." + kKeyOverlayEnterMarker;
	const std::string marker = doc_.hasKey(markerPath)
					   ? doc_.getString(markerPath)
					   : std::string(kDefaultEnterMarker);
	decoration.enter = OverlayEnter{rows[0], rows[1], facing, marker};
}

void OverlayTomlReader::readMarkerNpcs(WorldOverlay& overlay) const {
	if (!doc_.hasKey(kKeyOverlayMarkers)) {
		return;
	}
	for (const std::string& kind : doc_.subtableKeys(kKeyOverlayMarkers)) {
		const std::string path = std::string(kKeyOverlayMarkers) + "." +
					 kind + "." + kKeyOverlayNpc;
		overlay.markerNpcs[kind] = doc_.getString(path);
	}
}

void OverlayTomlReader::readMapElements(WorldOverlay& overlay) const {
	if (!doc_.hasKey(kKeyOverlayMaps)) {
		return;
	}
	const SceneTomlReader scenes(doc_);
	for (const std::string& mapId : doc_.subtableKeys(kKeyOverlayMaps)) {
		// Los elementos se validan contra las hojas del overlay.
		SceneDescription scratch;
		scenes.readExtraTileSets(scratch);
		scenes.readExtraElements(std::string(kKeyOverlayMaps) + "." +
						 mapId,
					 overlay.scale, scratch);
		overlay.mapElements[mapId] = std::move(scratch.elements);
	}
}

}
