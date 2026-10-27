#pragma once

#include <optional>
#include <vector>

#include "qt/services/WorldService.h"

// Dos mundos fijos, uno sin descripción. Si se construye con un error, todos
// los pedidos fallan con ese error.
class FakeWorldService : public WorldService {
	Q_OBJECT

public:
	explicit FakeWorldService(
		std::optional<ServiceError> failure = std::nullopt,
		QObject* parent = nullptr);

	void requestWorlds() override;

private:
	static std::vector<WorldSummary> fixedWorlds();

	std::optional<ServiceError> failure_;
};
