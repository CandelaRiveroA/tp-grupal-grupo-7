#include <QtGlobal>

#include "core/Application.h"

int main(int argc, char** argv) {
	qputenv("LIBGL_ALWAYS_SOFTWARE", "1");

	core::Application app(argc, argv);
	return app.run();
}
