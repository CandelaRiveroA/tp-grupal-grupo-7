#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "qt/services/WorldService.h"

class NetworkWorldService : public WorldService {
	Q_OBJECT

public:
	explicit NetworkWorldService(QObject* parent = nullptr);
	~NetworkWorldService() override;

	void requestWorlds() override;
	std::vector<WorldSummary> worlds() const override;

private:
	mutable std::mutex worldsMutex_;
	std::vector<WorldSummary> worlds_;
	std::thread worker_;

	void fetchWorlds();
};
