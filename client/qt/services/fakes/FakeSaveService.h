#pragma once

#include <optional>
#include <vector>

#include "qt/services/SaveService.h"

// Tres partidas fijas, una de ellas multiplayer. Si se construye con un
// error, todos los pedidos fallan con ese error.
class FakeSaveService : public SaveService {
	Q_OBJECT

public:
	explicit FakeSaveService(
		std::optional<ServiceError> failure = std::nullopt,
		QObject* parent = nullptr);

	void requestSaves() override;

private:
	static std::vector<SaveSummary> fixedSaves();

	std::optional<ServiceError> failure_;
};
