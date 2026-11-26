#include "FakeSaveService.h"

#include <QTimer>

#include <algorithm>
#include <vector>

#include "qt/services/fakes/FakeLatency.h"

FakeSaveService::FakeSaveService(std::optional<ServiceError> failure,
				 QObject* parent)
	: SaveService(parent)
	, saves_(buildInitialSaves())
	, failure_(failure) {}

std::vector<SaveSummary> FakeSaveService::buildInitialSaves() const {
	// Apariencias variadas dentro de los rangos del catalogo verificados por
	// el harness (hair 0..23, shirt 0..47, pants 0..6, accessory 0..19).
	SaveSummary iris;
	iris.id = 1;
	iris.worldName = "Valle Verde";
	iris.gameName = "Granja Yesi Farm";
	CharacterAppearance irisLook;
	irisLook.hair = 5;
	irisLook.hairColor = 9;
	irisLook.shirt = 2;
	irisLook.pants = 1;
	iris.players = {{"Iris", irisLook}};
	iris.multiplayer = false;
	iris.connectedPlayers = 1;
	iris.maxPlayers = 1;
	iris.inGameDate = "Día 3 de primavera, año 3";
	iris.gold = 404316;
	iris.playTimeSeconds = 196800;  // 54:40 hs

	SaveSummary coop;
	coop.id = 2;
	coop.worldName = "Valle Verde";
	coop.gameName = "Granja prueba";
	CharacterAppearance pruebaLook;
	pruebaLook.hair = 17;
	pruebaLook.shirt = 1;
	CharacterAppearance carlaLook;
	carlaLook.hair = 3;
	carlaLook.hairColor = 2;
	carlaLook.shirt = 4;
	coop.players = {{"prueba", pruebaLook}, {"Carla", carlaLook}};
	coop.multiplayer = true;
	coop.connectedPlayers = 2;
	coop.maxPlayers = 4;
	coop.inGameDate = "Día 1 de primavera, año 1";
	coop.gold = 500;
	coop.playTimeSeconds = 0;

	SaveSummary wake;
	wake.id = 3;
	wake.worldName = "Mundo del editor";
	wake.gameName = "Granja Bugisoft";
	CharacterAppearance wakeLook;
	wakeLook.hair = 12;
	wakeLook.hairColor = 3;
	wakeLook.shirt = 8;
	wakeLook.pants = 3;
	wakeLook.accessory = 1;
	wake.players = {{"Wake", wakeLook}};
	wake.multiplayer = false;
	wake.connectedPlayers = 1;
	wake.maxPlayers = 1;
	wake.inGameDate = "Día 23 de otoño, año 2";
	wake.gold = 149885;
	wake.playTimeSeconds = 267120;  // 74:12 hs

	return {iris, coop, wake};
}

void FakeSaveService::requestSaves(const std::string&) {
	QTimer::singleShot(kFakeDelayMs, this, [this]() {
		if (failure_) {
			emit savesFailed(*failure_);
			return;
		}
		emit savesReady(saves_);
	});
}

void FakeSaveService::deleteSave(std::uint32_t saveId) {
	QTimer::singleShot(kFakeDelayMs, this, [this, saveId]() {
		if (failure_) {
			emit deleteFailed(*failure_);
			return;
		}
		auto it = std::find_if(
			saves_.begin(), saves_.end(),
			[saveId](const SaveSummary& save) { return save.id == saveId; });
		if (it == saves_.end()) {
			emit deleteFailed(ServiceError::SaveNotFound);
			return;
		}
		saves_.erase(it);
		emit saveDeleted(saveId);
	});
}
