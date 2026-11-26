#include "NetworkWorldService.h"

#include <QMetaObject>
#include <Qt>
#include <exception>
#include <utility>

#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "common/common_socket.h"
#include "core/Constants.h"

NetworkWorldService::NetworkWorldService(QObject* parent) : WorldService(parent) {}

NetworkWorldService::~NetworkWorldService() {
	if (worker_.joinable()) {
		worker_.join();
	}
}

void NetworkWorldService::requestWorlds() {
	if (worker_.joinable()) {
		worker_.join();
	}
	{
		const std::lock_guard<std::mutex> lock(worldsMutex_);
		worlds_.clear();
	}
	worker_ = std::thread([this]() { fetchWorlds(); });
}

std::vector<WorldSummary> NetworkWorldService::worlds() const {
	const std::lock_guard<std::mutex> lock(worldsMutex_);
	return worlds_;
}

void NetworkWorldService::fetchWorlds() {
	try {
		Socket socket(core::kServerHost, core::kServerPort);
		socket.set_receive_timeout(5);
		ProtocolSender sender(socket);
		ProtocolReceiver receiver(socket);
		sender.send_world_list_request();
		const std::vector<std::string> names = receiver.receive_world_list();
		std::vector<WorldSummary> summaries;
		summaries.reserve(names.size());
		for (const std::string& name : names) {
			summaries.push_back(WorldSummary{name, ""});
		}
		{
			const std::lock_guard<std::mutex> lock(worldsMutex_);
			worlds_ = summaries;
		}
		QMetaObject::invokeMethod(
			this,
			[this, summaries = std::move(summaries)]() mutable {
				emit worldsReady(std::move(summaries));
			},
			Qt::QueuedConnection);
	} catch (const std::exception&) {
		QMetaObject::invokeMethod(this, [this]() { emit worldsFailed(ServiceError::Network); },
					  Qt::QueuedConnection);
	}
}
