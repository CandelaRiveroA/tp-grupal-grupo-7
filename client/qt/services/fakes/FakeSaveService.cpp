#include "FakeSaveService.h"

#include <QTimer>

#include "qt/services/fakes/FakeLatency.h"

FakeSaveService::FakeSaveService(std::optional<ServiceError> failure,
				 QObject* parent)
	: SaveService(parent)
	, failure_(failure) {}

std::vector<SaveSummary> FakeSaveService::fixedSaves() {
	SaveSummary solo;
	solo.id = 1;
	solo.worldId = 1;
	solo.gameName = "Granja del valle";
	solo.players = {{"Ana", CharacterAppearance{}}};
	solo.multiplayer = false;
	solo.inGameDate = "Primavera, día 12";
	solo.playTimeSeconds = 5400;

	SaveSummary coop;
	coop.id = 2;
	coop.worldId = 1;
	coop.gameName = "Cooperativa";
	coop.players = {{"Beto", CharacterAppearance{}},
			{"Carla", CharacterAppearance{}}};
	coop.multiplayer = true;
	coop.inGameDate = "Verano, día 3";
	coop.playTimeSeconds = 12600;

	SaveSummary reciente;
	reciente.id = 3;
	reciente.worldId = 2;
	reciente.gameName = "Prueba";
	reciente.players = {{"Dani", CharacterAppearance{}}};
	reciente.multiplayer = false;
	reciente.inGameDate = "Primavera, día 1";
	reciente.playTimeSeconds = 300;

	return {solo, coop, reciente};
}

void FakeSaveService::requestSaves() {
	QTimer::singleShot(kFakeDelayMs, this, [this]() {
		if (failure_) {
			emit savesFailed(*failure_);
			return;
		}
		emit savesReady(fixedSaves());
	});
}
