#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

#include "EditorWindow.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(app.style()->standardPalette());

	EditorWindow window(TALLER_SCENARIOS_DIR, TALLER_WORLDS_DIR);
	window.show();
	return app.exec();
}
