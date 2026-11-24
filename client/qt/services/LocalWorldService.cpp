#include "LocalWorldService.h"

#include <QTimer>
#include <utility>

#include "world_model/FormatError.h"
#include "world_model/WorldRepository.h"
#include "world_model/WorldValidator.h"

LocalWorldService::LocalWorldService(std::string worldsDir, QObject* parent)
	: WorldService(parent)
	, worldsDir_(std::move(worldsDir)) {}

std::vector<WorldSummary> LocalWorldService::worlds() const {
	const worldmodel::WorldRepository repository(worldsDir_);
	const worldmodel::WorldValidator validator;
	std::vector<WorldSummary> result;
	for (const std::string& name : repository.listWorlds()) {
		try {
			const worldmodel::World world = repository.load(name);
			if (validator.validate(world).empty()) {
				result.push_back(WorldSummary{world.name, ""});
			}
		} catch (const worldmodel::FormatError&) {
			// Los mundos corruptos o incompletos no se ofrecen en el lobby.
		}
	}
	return result;
}

void LocalWorldService::requestWorlds() {
	QTimer::singleShot(0, this, [this]() { emit worldsReady(worlds()); });
}
