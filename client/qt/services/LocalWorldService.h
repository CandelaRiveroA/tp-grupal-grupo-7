#pragma once

#include <string>
#include <vector>

#include "qt/services/WorldService.h"

// Lista los mundos válidos guardados en worlds/ (stand-in local hasta que
// el servidor entregue el catálogo de mundos por red).
class LocalWorldService : public WorldService {
	Q_OBJECT

public:
	explicit LocalWorldService(std::string worldsDir,
				   QObject* parent = nullptr);

	void requestWorlds() override;
	std::vector<WorldSummary> worlds() const override;

private:
	std::string worldsDir_;
};
