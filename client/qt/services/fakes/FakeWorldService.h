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

	// Mundos fijos, de forma sincronica: el shell los necesita antes de
	// mostrar la pantalla de personaje.
	std::vector<WorldSummary> worlds() const;

private:
	std::optional<ServiceError> failure_;
};
