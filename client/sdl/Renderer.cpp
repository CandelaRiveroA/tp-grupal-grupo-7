#include "Renderer.h"

#include "sdl/SdlContext.h"
#include "world/Player.h"
#include "world/Scenario.h"

namespace sdl {

namespace {

const SDL2pp::Color kPlayerColor(255, 165, 0, 255);
}

Renderer::Renderer(SdlContext& context)
	: context_(context) {}

void Renderer::render(const world::Scenario& scenario) {

	const world::Color& bg = scenario.background();
	context_.clear(SDL2pp::Color(bg.r, bg.g, bg.b, bg.a));

	const world::Player& player = scenario.player();
	context_.fillRect(SDL2pp::Rect(player.x(), player.y(),
				       player.width(), player.height()),
			  kPlayerColor);

	context_.present();
}

}
