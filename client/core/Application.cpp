#include "Application.h"

#include "qt/GameView.h"

namespace core {

namespace {

constexpr int kPlayerSize = 32;
constexpr int kPlayerSpeed = 5;
}

Application::Application(int& argc, char** argv)
	: config_(640, 480, "TP Final — Grupo 7")
	, app_(argc, argv)
	, input_()
	, window_(config_, input_)
	, player_((config_.width() - kPlayerSize) / 2,
		  (config_.height() - kPlayerSize) / 2,
		  kPlayerSize, kPlayerSize, kPlayerSpeed)
	, scenario_(config_, player_)
	, sdlContext_(config_)
	, renderer_(sdlContext_)
	, view_(nullptr)
	, loop_([this]() {

		int dx = 0;
		int dy = 0;
		if (input_.left()) dx -= player_.speed();
		if (input_.right()) dx += player_.speed();
		if (input_.up()) dy -= player_.speed();
		if (input_.down()) dy += player_.speed();
		if (dx != 0 || dy != 0) {
			player_.move(dx, dy, scenario_.width(), scenario_.height());
		}

		renderer_.render(scenario_);

		QImage& frame = view_->frameBuffer();
		sdlContext_.readPixels(frame.bits(),
				       static_cast<int>(frame.bytesPerLine()));
		view_->present();
	}) {
	window_.setContent(view_ = new qt::GameView(config_, &window_));
}

int Application::run() {
	window_.show();
	window_.setFocus();
	loop_.start();
	int rc = app_.exec();

	loop_.stop();
	return rc;
}

}
