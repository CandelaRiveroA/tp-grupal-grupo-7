#include "FakeWorldService.h"

#include <QTimer>

#include "qt/services/fakes/FakeLatency.h"

FakeWorldService::FakeWorldService(std::optional<ServiceError> failure,
				   QObject* parent)
	: WorldService(parent)
	, failure_(failure) {}

std::vector<WorldSummary> FakeWorldService::fixedWorlds() {
	WorldSummary valle;
	valle.id = 1;
	valle.name = "Valle Verde";
	valle.description =
		"Una granja tranquila junto al río, con cueva y pueblo cercanos.";

	WorldSummary sinDescripcion;
	sinDescripcion.id = 2;
	sinDescripcion.name = "Mundo del editor";

	return {valle, sinDescripcion};
}

void FakeWorldService::requestWorlds() {
	QTimer::singleShot(kFakeDelayMs, this, [this]() {
		if (failure_) {
			emit worldsFailed(*failure_);
			return;
		}
		emit worldsReady(fixedWorlds());
	});
}
