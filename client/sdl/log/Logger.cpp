#include "sdl/log/Logger.h"

#include <iostream>

namespace sdl {

namespace {

inline constexpr const char* kInfoPrefix = "[SDL][INFO] ";
inline constexpr const char* kWarningPrefix = "[SDL][WARNING] ";
inline constexpr const char* kErrorPrefix = "[SDL][ERROR] ";

const char* prefixFor(LogLevel level) {
	switch (level) {
	case LogLevel::Info:
		return kInfoPrefix;
	case LogLevel::Warning:
		return kWarningPrefix;
	case LogLevel::Error:
		return kErrorPrefix;
	}
	return kInfoPrefix;
}

}

void Logger::info(const std::string& message) const {
	log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) const {
	log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) const {
	log(LogLevel::Error, message);
}

void Logger::log(LogLevel level, const std::string& message) const {
	std::clog << prefixFor(level) << message << std::endl;
}

}
