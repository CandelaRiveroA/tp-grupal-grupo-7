#include "world_model/WorldTomlReader.h"

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "world_model/FormatError.h"
#include "world_model/TileCodec.h"

namespace worldmodel {

WorldTomlReader::WorldTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string WorldTomlReader::join(const std::string& base,
				  const std::string& key) const {
	return base + "." + key;
}

std::string WorldTomlReader::idx(const std::string& base,
				 std::size_t index) const {
	return base + "[" + std::to_string(index) + "]";
}

int WorldTomlReader::readPositiveInt(const std::string& path) const {
	const int value = doc_.getInt(path);
	if (value <= 0) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
	return value;
}

TilePos WorldTomlReader::readTilePos(const std::string& path) const {
	const std::array<int, 2> values = doc_.getIntArray<2>(path);
	return TilePos{values[0], values[1]};
}

World WorldTomlReader::read() const {
	const int version = doc_.getInt("format_version");
	if (version != 1) {
		throw FormatError(FormatErrorKind::InvalidValue,
				  "format_version");
	}

	World world;
	world.formatVersion = version;
	readWorld(world);
	readTilesets(world);
	for (const std::string& id : doc_.subtableKeys("maps")) {
		world.maps.emplace(id, readMap(world, id));
	}
	return world;
}

void WorldTomlReader::readWorld(World& world) const {
	world.slug = doc_.getString("world.slug");
	world.name = doc_.getString("world.name");
	if (doc_.hasKey("world.description")) {
		world.description = doc_.getString("world.description");
	}
}

void WorldTomlReader::readTilesets(World& world) const {
	for (const std::string& name : doc_.subtableKeys("tilesets")) {
		const std::string path = join("tilesets", name);
		const std::array<int, 2> cell =
			doc_.getIntArray<2>(join(path, "cell"));
		if (cell[0] <= 0 || cell[1] <= 0) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  join(path, "cell"));
		}
		world.tilesets.emplace(
			name, TilesetDef{doc_.getString(join(path, "image")),
					 cell[0], cell[1]});
	}
}

Map WorldTomlReader::readMap(const World& world,
			     const std::string& id) const {
	const std::string path = join("maps", id);
	Map map;
	map.id = id;

	const std::string kindPath = join(path, "kind");
	const std::optional<MapKind> kind =
		vocabulary_.mapKindFrom(doc_.getString(kindPath));
	if (!kind.has_value()) {
		throw FormatError(FormatErrorKind::InvalidValue, kindPath);
	}
	map.kind = *kind;

	if (map.kind == MapKind::Dungeon) {
		const std::string levelPath = join(path, "level");
		map.level = doc_.getInt(levelPath);
		// El rango 1..10 depende del id y lo valida WorldValidator.
		if (map.level < 0) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  levelPath);
		}
	}

	map.width = readPositiveInt(join(path, "width"));
	map.height = readPositiveInt(join(path, "height"));
	map.tileSize = readPositiveInt(join(path, "tile_size"));

	const std::size_t cells = static_cast<std::size_t>(map.width) *
				  static_cast<std::size_t>(map.height);
	const std::string basePath = join(path, "base_collision");
	map.baseCollision = doc_.getBoolArray(basePath);
	if (map.baseCollision.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, basePath);
	}
	const std::string collisionPath = join(path, "collision");
	map.collision = doc_.getBoolArray(collisionPath);
	if (map.collision.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, collisionPath);
	}

	map.ground = readGrid(world, join(path, "ground"), map.width,
			      map.height);
	const std::string pathsPath = join(path, "paths");
	if (doc_.hasKey(pathsPath)) {
		map.paths = readGrid(world, pathsPath, map.width, map.height);
	} else {
		map.paths.assign(cells, std::nullopt);
	}

	const std::string objectsPath = join(path, "objects");
	if (doc_.hasKey(objectsPath)) {
		readObjects(world, path, map);
	}
	const std::string markersPath = join(path, "markers");
	if (doc_.hasKey(markersPath)) {
		readMarkers(path, map);
	}
	const std::string portalsPath = join(path, "portals");
	if (doc_.hasKey(portalsPath)) {
		readPortals(path, map);
	}
	return map;
}

TileGrid WorldTomlReader::readGrid(const World& world,
				   const std::string& path, int width,
				   int height) const {
	const std::vector<std::string> texts = doc_.getStringArray(path);
	const std::size_t cells = static_cast<std::size_t>(width) *
				  static_cast<std::size_t>(height);
	if (texts.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}

	TileGrid grid;
	grid.reserve(cells);
	for (std::size_t i = 0; i < texts.size(); ++i) {
		const std::string cellPath = idx(path, i);
		const std::optional<TileRef> tile =
			TileCodec::decode(texts[i], cellPath);
		if (tile.has_value()) {
			checkTileset(world, *tile, cellPath);
		}
		grid.push_back(tile);
	}
	return grid;
}

void WorldTomlReader::checkTileset(const World& world, const TileRef& tile,
				   const std::string& path) const {
	if (world.tilesets.find(tile.tileset) == world.tilesets.end()) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
}

void WorldTomlReader::readObjects(const World& world,
				  const std::string& mapPath, Map& map) const {
	const std::string objectsPath = join(mapPath, "objects");
	const std::size_t count = doc_.arraySize(objectsPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = idx(objectsPath, i);
		MapObject object;

		const std::string rolePath = join(path, "role");
		const std::optional<ObjectRole> role =
			vocabulary_.objectRoleFrom(doc_.getString(rolePath));
		if (!role.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  rolePath);
		}
		object.role = *role;

		const std::string tilePath = join(path, "tile");
		const std::optional<TileRef> tile = TileCodec::decode(
			doc_.getString(tilePath), tilePath);
		if (!tile.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  tilePath);
		}
		checkTileset(world, *tile, tilePath);
		object.tile = *tile;

		object.cell = readTilePos(join(path, "cell"));

		const std::string sizePath = join(path, "size");
		if (doc_.hasKey(sizePath)) {
			const std::array<int, 2> size =
				doc_.getIntArray<2>(sizePath);
			object.width = size[0];
			object.height = size[1];
		}

		const std::string blockedPath = join(path, "blocked");
		if (doc_.hasKey(blockedPath)) {
			object.blocked = doc_.getBool(blockedPath);
		}

		const std::string doorPath = join(path, "door");
		if (doc_.hasKey(doorPath)) {
			object.door = readTilePos(doorPath);
		}

		map.objects.push_back(object);
	}
}

void WorldTomlReader::readMarkers(const std::string& mapPath,
				  Map& map) const {
	const std::string markersPath = join(mapPath, "markers");
	const std::size_t count = doc_.arraySize(markersPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = idx(markersPath, i);
		Marker marker;

		const std::string kindPath = join(path, "kind");
		const std::optional<MarkerKind> kind =
			vocabulary_.markerKindFrom(doc_.getString(kindPath));
		if (!kind.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  kindPath);
		}
		marker.kind = *kind;

		marker.cell = readTilePos(join(path, "cell"));
		map.markers.push_back(marker);
	}
}

void WorldTomlReader::readPortals(const std::string& mapPath,
				  Map& map) const {
	const std::string portalsPath = join(mapPath, "portals");
	const std::size_t count = doc_.arraySize(portalsPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = idx(portalsPath, i);
		Portal portal;

		const std::string kindPath = join(path, "kind");
		const std::optional<PortalKind> kind =
			vocabulary_.portalKindFrom(doc_.getString(kindPath));
		if (!kind.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  kindPath);
		}
		portal.kind = *kind;

		const std::string targetMapPath = join(path, "target_map");
		portal.targetMap = doc_.getString(targetMapPath);
		if (portal.targetMap.empty()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  targetMapPath);
		}

		portal.cell = readTilePos(join(path, "cell"));
		portal.targetCell = readTilePos(join(path, "target_cell"));
		map.portals.push_back(portal);
	}
}

}
