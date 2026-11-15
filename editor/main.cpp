#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>

#include "EditorWindow.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	// setColorScheme existe desde Qt 6.5; con Qt viejo el estilo Fusion
	// igual se ve claro (no se fuerza el esquema de color).
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(app.style()->standardPalette());

	EditorWindow window(TALLER_SCENARIOS_DIR, TALLER_WORLDS_DIR);
	window.show();
	return app.exec();
}
