#include "world_model/ScenarioTomlReader.h"

#include <cstddef>
#include <optional>

#include "world_model/FormatError.h"

namespace worldmodel {

ScenarioTomlReader::ScenarioTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc), fields_(doc) {}

Scenario ScenarioTomlReader::read() const {
	fields_.readFormatVersion();

	Scenario scenario;
	scenario.tilesets = fields_.readTilesets("tilesets");

	scenario.id = doc_.getString(fields_.join("scenario", "id"));
	const std::string kindPath = fields_.join("scenario", "kind");
	const std::optional<MapKind> kind =
		vocabulary_.mapKindFrom(doc_.getString(kindPath));
	if (!kind.has_value()) {
		throw FormatError(FormatErrorKind::InvalidValue, kindPath);
	}
	scenario.kind = *kind;

	scenario.width =
		fields_.readPositiveInt(fields_.join("scenario", "width"));
	scenario.height =
		fields_.readPositiveInt(fields_.join("scenario", "height"));
	scenario.tileSize = fields_.readPositiveInt(
		fields_.join("scenario", "tile_size"));

	const std::size_t cells = static_cast<std::size_t>(scenario.width) *
				  static_cast<std::size_t>(scenario.height);
	const std::string collisionPath =
		fields_.join("scenario", "collision");
	scenario.collision = doc_.getBoolArray(collisionPath);
	if (scenario.collision.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue,
				  collisionPath);
	}

	scenario.ground = fields_.readGrid(
		fields_.join("scenario", "ground"), scenario.width,
		scenario.height, scenario.tilesets);
	const std::string frontPath = fields_.join("scenario", "front");
	if (doc_.hasKey(frontPath)) {
		scenario.front = fields_.readGrid(frontPath, scenario.width,
						  scenario.height,
						  scenario.tilesets);
	} else {
		scenario.front.assign(cells, std::nullopt);
	}
	return scenario;
}

}
