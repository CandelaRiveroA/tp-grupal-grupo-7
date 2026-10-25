#pragma once

#include <QObject>

#include <vector>

#include "common/model/draft/SaveSummary.h"
#include "qt/services/ServiceError.h"

// Partidas guardadas. El pedido es asíncrono: requestSaves() dispara y el
// resultado llega por savesReady() o savesFailed().
class SaveService : public QObject {
	Q_OBJECT

public:
	explicit SaveService(QObject* parent = nullptr);
	~SaveService() override;

	virtual void requestSaves() = 0;

signals:
	void savesReady(std::vector<SaveSummary> saves);
	void savesFailed(ServiceError error);
};
