#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QStyle>
#include <QStyleFactory>
#include <QtGlobal>

#include "Constants.h"
#include "EditorWindow.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(app.style()->standardPalette());

	for (const char* path :
	     {":/fonts/stardew.ttf", ":/fonts/pixel_operator.ttf"}) {
		if (QFontDatabase::addApplicationFont(path) < 0) {
			qWarning("No se pudo registrar la fuente %s", path);
		}
	}
	QFont font("Pixel Operator");
	font.setPixelSize(editor::kBaseFontPx);
	app.setFont(font);

	QFile qss(":/editor/editor.qss");
	if (!qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning("No se pudo abrir el recurso :/editor/editor.qss");
	} else {
		app.setStyleSheet(QString::fromUtf8(qss.readAll()));
	}

	EditorWindow window(TALLER_SCENARIOS_DIR, TALLER_WORLDS_DIR);
	window.show();
	return app.exec();
}
