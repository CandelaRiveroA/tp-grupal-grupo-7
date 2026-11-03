#include "FakeSessionService.h"

#include <QTimer>

#include "qt/services/fakes/FakeLatency.h"

FakeSessionService::FakeSessionService(std::optional<ServiceError> failure,
				       QObject* parent)
	: SessionService(parent)
	, failure_(failure) {}

void FakeSessionService::createGame(const GameSelection&) {
	QTimer::singleShot(kFakeDelayMs, this, [this]() {
		if (failure_) {
			emit sessionFailed(*failure_);
			return;
		}
		emit gameCreated();
	});
}

void FakeSessionService::joinGame(const std::string& code, const std::string&, const std::string&) {
	QTimer::singleShot(kFakeDelayMs, this, [this, code]() {
		if (failure_) {
			emit sessionFailed(*failure_);
			return;
		}
		if (code == kFullGameCode) {
			emit sessionFailed(ServiceError::GameFull);
			return;
		}
		if (code != kValidCode) {
			emit sessionFailed(ServiceError::InvalidCode);
			return;
		}
		emit gameJoined(kJoinedGameName, kJoinedWorldId);
	});
}

void FakeSessionService::loadGame(std::uint32_t) {
	QTimer::singleShot(kFakeDelayMs, this, [this]() {
		if (failure_) {
			emit sessionFailed(*failure_);
			return;
		}
		emit gameLoaded();
	});
}
