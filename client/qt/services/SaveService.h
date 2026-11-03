#pragma once

#include <QObject>

#include <string>
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

	virtual void requestSaves(const std::string& username) = 0;
	virtual void deleteSave(std::uint32_t saveId) = 0;

signals:
	void savesReady(std::vector<SaveSummary> saves);
	void savesFailed(ServiceError error);
	void saveDeleted(std::uint32_t saveId);
	void deleteFailed(ServiceError error);
};
