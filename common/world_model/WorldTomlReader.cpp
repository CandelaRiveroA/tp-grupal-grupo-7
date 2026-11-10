#include "world_model/WorldTomlReader.h"

#include <array>
#include <optional>

#include "world_model/FormatError.h"
#include "world_model/TileCodec.h"

namespace worldmodel {

WorldTomlReader::WorldTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc), fields_(doc) {}

World WorldTomlReader::read() const {
	World world;
	world.formatVersion = fields_.readFormatVersion();
	readWorld(world);
	world.tilesets = fields_.readTilesets("tilesets");
	for (const std::string& id : doc_.subtableKeys("maps")) {
		world.maps.emplace(id, readMap(world, id));
	}
	return world;
}

void WorldTomlReader::readWorld(World& world) const {
	world.slug = doc_.getString("world.slug");
	world.name = doc_.getString("world.name");
}

Map WorldTomlReader::readMap(const World& world,
			     const std::string& id) const {
	const std::string path = fields_.join("maps", id);
	Map map;
	map.id = id;

	const std::string kindPath = fields_.join(path, "kind");
	const std::optional<MapKind> kind =
		vocabulary_.mapKindFrom(doc_.getString(kindPath));
	if (!kind.has_value()) {
		throw FormatError(FormatErrorKind::InvalidValue, kindPath);
	}
	map.kind = *kind;

	if (map.kind == MapKind::Dungeon) {
		const std::string levelPath = fields_.join(path, "level");
		map.level = doc_.getInt(levelPath);
		// El rango 1..10 depende del id y lo valida WorldValidator.
		if (map.level < 0) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  levelPath);
		}
	}

	map.width =
		fields_.readPositiveInt(fields_.join(path, "width"));
	map.height =
		fields_.readPositiveInt(fields_.join(path, "height"));
	map.tileSize =
		fields_.readPositiveInt(fields_.join(path, "tile_size"));

	const std::size_t cells = static_cast<std::size_t>(map.width) *
				  static_cast<std::size_t>(map.height);
	const std::string basePath =
		fields_.join(path, "base_collision");
	map.baseCollision = doc_.getBoolArray(basePath);
	if (map.baseCollision.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, basePath);
	}
	const std::string collisionPath =
		fields_.join(path, "collision");
	map.collision = doc_.getBoolArray(collisionPath);
	if (map.collision.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue,
				  collisionPath);
	}

	map.ground = fields_.readGrid(fields_.join(path, "ground"),
				      map.width, map.height,
				      world.tilesets);
	const std::string pathsPath = fields_.join(path, "paths");
	if (doc_.hasKey(pathsPath)) {
		map.paths = fields_.readGrid(pathsPath, map.width,
					     map.height, world.tilesets);
	} else {
		map.paths.assign(cells, std::nullopt);
	}

	const std::string objectsPath = fields_.join(path, "objects");
	if (doc_.hasKey(objectsPath)) {
		readObjects(world, path, map);
	}
	const std::string markersPath = fields_.join(path, "markers");
	if (doc_.hasKey(markersPath)) {
		readMarkers(path, map);
	}
	const std::string portalsPath = fields_.join(path, "portals");
	if (doc_.hasKey(portalsPath)) {
		readPortals(path, map);
	}
	return map;
}

void WorldTomlReader::readObjects(const World& world,
				  const std::string& mapPath,
				  Map& map) const {
	const std::string objectsPath = fields_.join(mapPath, "objects");
	const std::size_t count = doc_.arraySize(objectsPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = fields_.idx(objectsPath, i);
		MapObject object;

		const std::string rolePath = fields_.join(path, "role");
		const std::optional<ObjectRole> role =
			vocabulary_.objectRoleFrom(doc_.getString(rolePath));
		if (!role.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  rolePath);
		}
		object.role = *role;

		const std::string tilePath = fields_.join(path, "tile");
		const std::optional<TileRef> tile = TileCodec::decode(
			doc_.getString(tilePath), tilePath);
		if (!tile.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  tilePath);
		}
		fields_.checkTileset(world.tilesets, *tile, tilePath);
		object.tile = *tile;

		object.cell = fields_.readTilePos(fields_.join(path, "cell"));

		const std::string sizePath = fields_.join(path, "size");
		if (doc_.hasKey(sizePath)) {
			const std::array<int, 2> size =
				doc_.getIntArray<2>(sizePath);
			object.width = size[0];
			object.height = size[1];
		}

		const std::string blockedPath =
			fields_.join(path, "blocked");
		if (doc_.hasKey(blockedPath)) {
			object.blocked = doc_.getBool(blockedPath);
		}

		const std::string doorPath = fields_.join(path, "door");
		if (doc_.hasKey(doorPath)) {
			object.door = fields_.readTilePos(doorPath);
		}

		map.objects.push_back(object);
	}
}

void WorldTomlReader::readMarkers(const std::string& mapPath,
				  Map& map) const {
	const std::string markersPath = fields_.join(mapPath, "markers");
	const std::size_t count = doc_.arraySize(markersPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = fields_.idx(markersPath, i);
		Marker marker;

		const std::string kindPath = fields_.join(path, "kind");
		const std::optional<MarkerKind> kind =
			vocabulary_.markerKindFrom(doc_.getString(kindPath));
		if (!kind.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  kindPath);
		}
		marker.kind = *kind;

		marker.cell = fields_.readTilePos(fields_.join(path, "cell"));
		map.markers.push_back(marker);
	}
}

void WorldTomlReader::readPortals(const std::string& mapPath,
				  Map& map) const {
	const std::string portalsPath = fields_.join(mapPath, "portals");
	const std::size_t count = doc_.arraySize(portalsPath);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string path = fields_.idx(portalsPath, i);
		Portal portal;

		const std::string kindPath = fields_.join(path, "kind");
		const std::optional<PortalKind> kind =
			vocabulary_.portalKindFrom(doc_.getString(kindPath));
		if (!kind.has_value()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  kindPath);
		}
		portal.kind = *kind;

		const std::string targetMapPath =
			fields_.join(path, "target_map");
		portal.targetMap = doc_.getString(targetMapPath);
		if (portal.targetMap.empty()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  targetMapPath);
		}

		portal.cell = fields_.readTilePos(fields_.join(path, "cell"));
		portal.targetCell =
			fields_.readTilePos(fields_.join(path, "target_cell"));
		map.portals.push_back(portal);
	}
}

}
