#pragma once

#include <QApplication>

#include "GameLoop.h"
#include "InputManager.h"
#include "WindowConfig.h"
#include "qt/MainWindow.h"
#include "sdl/Renderer.h"
#include "sdl/SdlContext.h"
#include "world/Player.h"
#include "world/Scenario.h"

namespace qt {
class GameView;
}

namespace core {

class Application {
public:
	Application(int& argc, char** argv);

	int run();

private:
	WindowConfig config_;
	QApplication app_;
	core::InputManager input_;
	qt::MainWindow window_;
	world::Player player_;
	world::Scenario scenario_;
	sdl::SdlContext sdlContext_;
	sdl::Renderer renderer_;
	qt::GameView* view_;
	core::GameLoop loop_;
};

}
