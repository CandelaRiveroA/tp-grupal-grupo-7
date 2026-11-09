#pragma once

#include <optional>
#include <vector>

#include "qt/services/SaveService.h"

// Tres partidas iniciales, una multiplayer. Con error, todo pedido falla;
// deleteSave() persiste en los requestSaves() siguientes.
class FakeSaveService : public SaveService {
	Q_OBJECT

public:
	explicit FakeSaveService(
		std::optional<ServiceError> failure = std::nullopt,
		QObject* parent = nullptr);

	void requestSaves(const std::string& username) override;
	void deleteSave(std::uint32_t saveId) override;

private:
	std::vector<SaveSummary> buildInitialSaves() const;

	std::vector<SaveSummary> saves_;
	std::optional<ServiceError> failure_;
};
