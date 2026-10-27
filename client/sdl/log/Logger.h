#pragma once

#include <string>

namespace sdl {

enum class LogLevel {
	Info,
	Warning,
	Error,
};

class Logger {
public:
	void info(const std::string& message) const;
	void warning(const std::string& message) const;
	void error(const std::string& message) const;

private:
	void log(LogLevel level, const std::string& message) const;
};

}
