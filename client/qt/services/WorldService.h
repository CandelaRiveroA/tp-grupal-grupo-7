#pragma once

#include <QObject>

#include <vector>

#include "common/model/draft/WorldSummary.h"
#include "qt/services/ServiceError.h"

// Mundos disponibles. El pedido es asíncrono: requestWorlds() dispara y el
// resultado llega por worldsReady() o worldsFailed().
class WorldService : public QObject {
	Q_OBJECT

public:
	explicit WorldService(QObject* parent = nullptr);
	~WorldService() override;

	virtual void requestWorlds() = 0;
	virtual std::vector<WorldSummary> worlds() const = 0;

signals:
	void worldsReady(std::vector<WorldSummary> worlds);
	void worldsFailed(ServiceError error);
};
