#pragma once

namespace world {
class Scenario;
}

namespace sdl {
class SdlContext;
}

namespace sdl {

class Renderer {
public:
	explicit Renderer(SdlContext& context);

	void render(const world::Scenario& scenario);

private:
	SdlContext& context_;
};

}
