#pragma once

#include <string>

namespace core {

enum class CloseBehavior {
	StopLoopThenQuit
};

class WindowConfig {
public:
	WindowConfig(int width, int height, std::string title);

	int width() const;
	int height() const;
	const std::string& title() const;
	CloseBehavior closeBehavior() const;

private:
	int width_;
	int height_;
	std::string title_;
	CloseBehavior closeBehavior_;
};

}
