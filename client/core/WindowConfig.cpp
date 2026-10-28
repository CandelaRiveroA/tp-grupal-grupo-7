#include "WindowConfig.h"

#include <utility>

namespace core {

WindowConfig::WindowConfig(int width, int height, std::string title)
	: width_(width)
	, height_(height)
	, title_(std::move(title))
	, closeBehavior_(CloseBehavior::StopLoopThenQuit) {}

int WindowConfig::width() const { return width_; }

int WindowConfig::height() const { return height_; }

const std::string& WindowConfig::title() const { return title_; }

CloseBehavior WindowConfig::closeBehavior() const { return closeBehavior_; }

}
