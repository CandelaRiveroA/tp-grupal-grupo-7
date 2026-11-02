#include <QAbstractButton>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFontDatabase>
#include <QFontInfo>
#include <QFontMetrics>
#include <QSet>
#include <QSlider>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QLayout>
#include <QLineEdit>
#include <QPalette>
#include <QTimer>
#include <QPixmap>

#include <algorithm>
#include <QDir>

#include <cstdio>

// Harness de la pantalla de personaje: compila los widgets reales y los
// ejercita offscreen. Ver client/qt/harness/README.md.
//   QT_QPA_PLATFORM=offscreen ./qt_harness_screen
// Variables: HARNESS_OUT (carpeta de salida, por defecto ./harness-out),
// FONT_PX (prueba de tamano de fuente), BASELINE (ruta de una captura),
// C8_SHOTS (carpeta para las 4 capturas de estado).
namespace {
QString outDir() {
	const QString dir = qEnvironmentVariable("HARNESS_OUT", QDir::currentPath() + "/harness-out");
	QDir().mkpath(dir);
	return dir;
}
}  // namespace
#include <optional>

#include "qt/screens/CharacterScreen.h"
#include "qt/services/CharacterSpriteComposer.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CharacterPaletteLoader.h"
#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "qt/services/fakes/FakeSessionService.h"
#include "qt/theme/CharacterRects.h"
#include "qt/theme/CharacterSprites.h"
#include "qt/widgets/InputBox.h"

namespace {
int failures = 0;

// Fuente de capas real (las hojas de Stardew), cargada como en la app.
class HarnessSource: public CharacterSpriteComposer {
public:
	HarnessSource(): CharacterSpriteComposer(loadCharacters(), loadPalettes()) {}

private:
	static world::CharacterCatalog loadCharacters() {
		return world::CharacterCatalogLoader().loadFromFile(
			std::string(TALLER_SOURCE_DIR) + "/client/sdl/assets/characters.toml");
	}
	static world::CharacterPalettes loadPalettes() {
		QFile file(":/character/palettes.toml");
		file.open(QIODevice::ReadOnly);
		return world::CharacterPaletteLoader().loadFromString(file.readAll().toStdString());
	}
};

// Musica de prueba: registra la ultima pista pedida.
class RecordingMusic: public core::MusicControl {
public:
	void play(core::MusicTrack track) override { last = track; ++calls; }
	void stop() override { ++stops; }
	core::MusicTrack last = core::MusicTrack::MenuTheme;
	int calls = 0;
	int stops = 0;
};

// Sesion instrumentada: cuenta los pedidos y responde con delay o error.
class CountingSession: public SessionService {
public:
	explicit CountingSession(int delayMs = 20, std::optional<ServiceError> error = std::nullopt,
				 QObject* parent = nullptr)
		: SessionService(parent), delay_(delayMs), error_(error) {}
	void createGame(const GameSelection&) override {
		++creates;
		QTimer::singleShot(delay_, this, [this]() {
			if (error_) { emit sessionFailed(*error_); return; }
			emit gameCreated();
		});
	}
	void joinGame(const std::string&) override { ++joins; }
	void loadGame(std::uint32_t) override {}
	int creates = 0;
	int joins = 0;
private:
	int delay_;
	std::optional<ServiceError> error_;
};
void check(bool ok, const char* name) {
	std::printf("%-62s %s\n", name, ok ? "PASS" : "FAIL");
	if (!ok) ++failures;
}
void settle() {
	QEventLoop loop;
	QTimer::singleShot(80, &loop, &QEventLoop::quit);
	loop.exec();
}
}  // namespace

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QFontDatabase::addApplicationFont(":/fonts/stardew.ttf");
	QFontDatabase::addApplicationFont(":/fonts/pixel_operator.ttf");
	QFile qss(":/ui.qss");
	if (qss.open(QIODevice::ReadOnly)) {
		QString sheet = QString::fromUtf8(qss.readAll());
		if (qEnvironmentVariableIsSet("FONT_PX")) {  // FONT_PX: prueba de tamano de PixelOperator
			const QString px = qEnvironmentVariable("FONT_PX");
			sheet.replace("font-family: \"StardewSmall\";\n    font-size: 24px;",
				      "font-family: \"Pixel Operator\";\n    font-size: " + px + "px;");
			QFontDatabase::addApplicationFont(":/fonts/pixel_operator.ttf");
		}
		app.setStyleSheet(sheet);
	}

	HarnessSource catalog;
	FakeSessionService session;
	CharacterScreen screen(&session, &catalog);
	WorldSummary world;
	world.id = 1;
	world.name = "Granja Estandar";
	screen.startNewGame(world);
	screen.resize(1280, 720);
	screen.show();
	screen.onEnter();
	settle();

	{  // c2-measure
		QWidget* pc = screen.findChild<QWidget*>("previewColumn");
		QWidget* fc = screen.findChild<QWidget*>("formColumn");
		QWidget* win = screen.findChild<QWidget*>("characterWindow");
		auto dump = [](const char* n, QWidget* w) {
			if (!w) { std::fprintf(stderr, "[m] %s ausente\n", n); return; }
			std::fprintf(stderr, "[m] %-14s size=%dx%d hint=%dx%d minHint=%dx%d\n", n,
				     w->width(), w->height(), w->sizeHint().width(), w->sizeHint().height(),
				     w->minimumSizeHint().width(), w->minimumSizeHint().height());
		};
		dump("previewColumn", pc);
		dump("formColumn", fc);
		dump("characterWindow", win);
		if (win && win->layout()) {
			const QSize m = win->layout()->minimumSize();
			std::fprintf(stderr, "[m] window layout minimumSize=%dx%d\n", m.width(), m.height());
		}
	}
	{  // status-visible
		auto* st = screen.findChild<QLabel*>("screenStatus");
		auto* win2 = screen.findChild<QWidget*>("characterWindow");
		auto* bk = screen.findChild<QWidget*>("backButton");
		if (st && win2 && bk) {
			st->setText("Ya existe una partida con ese nombre.");
			st->setVisible(true);
			settle();
			check(!win2->geometry().intersects(st->geometry()) &&
				      !win2->geometry().intersects(bk->geometry()) &&
				      win2->geometry().top() >= 0,
			      "ventana no pisa estado visible ni Volver (720 px)");
			std::fprintf(stderr, "[m] win=%d..%d status=%d..%d back=%d..%d\n",
				     win2->geometry().top(), win2->geometry().bottom(),
				     st->geometry().top(), st->geometry().bottom(),
				     bk->geometry().top(), bk->geometry().bottom());
			st->setVisible(false);
			settle();
		}
	}
	{  // c5-checks
		auto* eyes = screen.findChild<QSlider*>("eyesSlider");
		auto* hair = screen.findChild<QSlider*>("hairColorSlider");
		check(eyes != nullptr && hair != nullptr, "sliders eyesSlider/hairColorSlider");
		check(eyes && hair && eyes->cursor().shape() == hair->cursor().shape() &&
			      eyes->cursor().shape() != Qt::ArrowCursor,
		      "sliders con el cursor del juego");
		QLabel* sample = nullptr;
		for (QLabel* l : screen.findChildren<QLabel*>()) {
			if (l->text() == "Nombre") sample = l;
		}
		if (sample) sample->ensurePolished();
		check(sample && QFontInfo(sample->font()).family() == "Pixel Operator",
		      "etiquetas bajo #character usan Pixel Operator");
		check(sample && sample->font().pixelSize() == 24, "etiquetas a 24 px (multiplo de la grilla)");
		auto* ci = screen.findChild<QLineEdit*>("characterInput");
		auto* gi = screen.findChild<QLineEdit*>("gameInput");
		if (ci) ci->ensurePolished();
		if (gi) gi->ensurePolished();
		check(ci && gi && QFontInfo(ci->font()).family() == "Pixel Operator" &&
			      QFontInfo(gi->font()).family() == "Pixel Operator" &&
			      ci->font().pixelSize() == 24,
		      "inputs usan Pixel Operator a 24 px");
		// El limite busca que entren los nombres comunes, no el peor caso:
		// con letras anchas (M) el campo hace scroll.
		const QString mixed = "Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn";
		const QString common(CHAR_INPUT_MAX_CHARS, QChar('a'));
		const QString wide(CHAR_INPUT_MAX_CHARS, QChar('M'));
		auto widthOf = [](QLineEdit* e, const QString& t) {
			return e ? e->fontMetrics().horizontalAdvance(t) : -1;
		};
		std::fprintf(stderr, "[m] hueco=%d  %d comunes=%d px  %d anchas=%d px  mezclado(%d)=%d px\n",
			     ci ? ci->width() : 0, CHAR_INPUT_MAX_CHARS, widthOf(ci, common),
			     CHAR_INPUT_MAX_CHARS, widthOf(ci, wide), int(mixed.size()), widthOf(ci, mixed));
		check(ci && widthOf(ci, common) <= ci->width(),
		      "el limite de caracteres entra con letras comunes");
		check(ci && gi && ci->maxLength() == CHAR_INPUT_MAX_CHARS &&
			      gi->maxLength() == CHAR_INPUT_MAX_CHARS,
		      "los dos campos usan CHAR_INPUT_MAX_CHARS");
		{  // slider-paint
			auto* sl = screen.findChild<QSlider*>("eyesSlider");
			if (sl) {
				const QImage img = sl->grab().toImage();
				int orange = 0, dark = 0;
				QSet<int> lineRows;
				for (int y = 0; y < img.height(); ++y)
					for (int x = 0; x < img.width(); ++x) {
						const QColor c = img.pixelColor(x, y);
						if (c.red() == 0xdc && c.green() == 0x7b && c.blue() == 0x05) ++orange;
						if (c.red() == 0x4a && c.green() == 0x2a && c.blue() == 0x1c) {
							++dark;
							lineRows.insert(y);
						}
					}
				std::fprintf(stderr, "[m] slider %dx%d naranja=%d linea_oscura=%d filas_de_linea=%d\n",
					     img.width(), img.height(), orange, dark, lineRows.size());
				check(orange > 0, "slider dibuja la perilla (naranja del sprite)");
				check(lineRows.size() == 3, "el carril es una linea oscura de 3 px de alto");
				// La perilla es el sprite: contorno #5b2b2a + naranjas. La
				// linea usa otro marron (#4a2a1c), asi que no se confunden.
				int knobTop = 999, knobBottom = -1, knobLeft = 999, knobRight = -1;
				for (int y = 0; y < img.height(); ++y)
					for (int x = 0; x < img.width(); ++x) {
						const QColor c = img.pixelColor(x, y);
						const bool isKnob =
							(c.red() == 0xdc && c.green() == 0x7b && c.blue() == 0x05) ||
							(c.red() == 0xb1 && c.green() == 0x4e && c.blue() == 0x05) ||
							(c.red() == 0x5b && c.green() == 0x2b && c.blue() == 0x2a);
						if (isKnob) {
							knobTop = std::min(knobTop, y);
							knobBottom = std::max(knobBottom, y);
							knobLeft = std::min(knobLeft, x);
							knobRight = std::max(knobRight, x);
						}
					}
				std::fprintf(stderr, "[m] perilla y=%d..%d (alto %d) x=%d..%d (ancho %d)\n", knobTop,
					     knobBottom, knobBottom - knobTop + 1, knobLeft, knobRight,
					     knobRight - knobLeft + 1);
				check(knobBottom - knobTop + 1 == 14 && knobRight - knobLeft + 1 == 11,
				      "la perilla mide 11x14 (sprite chico)");
				const int before = sl->value();
				sl->setValue(sl->maximum());
				QApplication::processEvents();
				const QImage moved = sl->grab().toImage();
				check(moved != img || before == sl->maximum(), "la perilla se mueve con el valor");
				sl->setValue(before);
			}
		}
		QFile src(QString(TALLER_SOURCE_DIR) + "/client/qt/screens/CharacterScreen.cpp");
		bool ascii = src.open(QIODevice::ReadOnly);
		if (ascii) {
			const QString code = QString::fromUtf8(src.readAll());
			ascii = !code.contains(QChar(0x2026));
		}
		check(ascii, "sin U+2026 (la fuente no tiene el glifo)");
	}
	{  // row-geometry
		QWidget* pc = screen.findChild<QWidget*>("previewColumn");
		QWidget* fc = screen.findChild<QWidget*>("formColumn");
		QWidget* win = screen.findChild<QWidget*>("characterWindow");
		if (pc && fc && win) {
			std::fprintf(stderr, "[g] ventana %dx%d  previewColumn y=%d..%d (alto %d)  formColumn y=%d..%d (alto %d)\n",
				     win->width(), win->height(), pc->geometry().top(), pc->geometry().bottom(), pc->height(),
				     fc->geometry().top(), fc->geometry().bottom(), fc->height());
			for (QWidget* child : pc->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
				std::fprintf(stderr, "[g]   hijo %-12s y=%3d..%3d (alto %3d) x=%d..%d\n", child->metaObject()->className(),
					     child->geometry().top(), child->geometry().bottom(), child->height(), child->geometry().left(),
					     child->geometry().right());
			const QRect inner = win->rect().adjusted(24, 24, -24, -20);
			std::fprintf(stderr, "[g] area util de la ventana y=%d..%d (alto %d)\n", inner.top(), inner.bottom(), inner.height());
		}
	}
	{  // c9-checks: distribucion de la columna izquierda y slider de pantalon
		QWidget* pc = screen.findChild<QWidget*>("previewColumn");
		auto* pants = screen.findChild<QSlider*>("pantsColorSlider");
		check(pants != nullptr, "slider pantsColorSlider existe");
		check(pants && pants->cursor().shape() != Qt::ArrowCursor, "pantsColorSlider con el cursor del juego");
		check(pants && pants->maximum() == catalog.count(CharacterLayer::PantsColor) - 1 && catalog.count(CharacterLayer::PantsColor) > 1,
		      "pantsColorSlider cubre todos los colores del catalogo");
		if (pc) {
			QList<QRect> items;
			for (QWidget* child : pc->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
				items.append(child->geometry());
			std::sort(items.begin(), items.end(), [](const QRect& a, const QRect& b) { return a.top() < b.top(); });
			check(items.size() == 6, "columna izquierda: retrato + 5 filas");
			bool uniform = items.size() == 6;
			int gap0 = -1;
			for (int i = 1; i < items.size() && uniform; ++i) {
				const int gap = items[i].top() - items[i - 1].bottom() - 1;
				std::fprintf(stderr, "[g] hueco %d = %d px\n", i, gap);
				if (gap0 < 0) gap0 = gap;
				uniform = uniform && gap == gap0;
			}
			check(uniform, "separacion identica entre el retrato y las filas (tolerancia 0)");
			const int bottomFree = pc->rect().bottom() - (items.isEmpty() ? 0 : items.last().bottom());
			std::fprintf(stderr, "[g] espacio libre bajo la ultima fila = %d px\n", bottomFree);
			check(bottomFree <= 1, "la ultima fila llega al borde inferior de la columna (<= 1 px)");
					int rowHeights = 0;
		for (QWidget* c : pc->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
			if (qstrcmp(c->metaObject()->className(), "CycleRow") == 0) rowHeights += c->height();
		check(rowHeights == 5 * 48, "las 5 filas miden 48 px cada una (nombre arriba, numero abajo)");
		}
	}
	CharacterSprites sprites;
	const QPixmap sheet(":/ui/character/text_box.png");
	check(sheet.size() == QSize(192, 48), "text_box.png 192x48");
	check(sprites.inputBox().size() ==
		      QSize(CHAR_INPUT_LOGICAL_W * CHAR_INPUT_SCALE,
			    CHAR_INPUT_LOGICAL_H * CHAR_INPUT_SCALE),
	      "inputBox 204x36 (68x12 a x3)");

	const int cw = CHAR_INPUT_MARGIN_LEFT * CHAR_INPUT_SCALE;
	const int chh = CHAR_INPUT_MARGIN_TOP * CHAR_INPUT_SCALE;
	const QImage expected =
		sheet.scaled(48, 12, Qt::IgnoreAspectRatio, Qt::FastTransformation)
			.copy(0, 0, CHAR_INPUT_MARGIN_LEFT, CHAR_INPUT_MARGIN_TOP)
			.scaled(cw, chh, Qt::IgnoreAspectRatio, Qt::FastTransformation)
			.toImage();
	check(sprites.inputBox().toImage().copy(0, 0, cw, chh) == expected,
	      "inputBox: esquina sup-izq intacta (9-slice no deforma)");

	QList<InputBox*> boxes;
	for (QLabel* label : screen.findChildren<QLabel*>()) {
		if (auto* box = dynamic_cast<InputBox*>(label)) boxes.append(box);
	}
	for (InputBox* b : boxes) {  // geo-debug
		QWidget* col = b->parentWidget();
		std::fprintf(stderr, "[geo] box %dx%d visibleRegion=%d parent=%s %dx%d\n",
			     b->width(), b->height(), b->visibleRegion().boundingRect().width(),
			     col ? col->objectName().toUtf8().constData() : "-",
			     col ? col->width() : 0, col ? col->height() : 0);
	}
	check(boxes.size() == 2, "dos InputBox en el formulario");
	bool boxesOk = boxes.size() == 2;
	for (InputBox* box : boxes) {
		boxesOk = boxesOk && box->size() == sprites.inputBox().size() &&
			  box->edit() != nullptr && !box->edit()->hasFrame();
	}
	check(boxesOk, "InputBox: tamano del sprite y QLineEdit sin marco");

	{
		bool inside = !boxes.isEmpty();
		bool labelAbove = !boxes.isEmpty();
		for (InputBox* b : boxes) {
			QWidget* col = b->parentWidget();
			inside = inside && col->rect().contains(b->geometry());
			// la etiqueta ("Nombre"/"Partida") queda arriba de la caja
			bool found = false;
			for (QLabel* l : col->findChildren<QLabel*>()) {
				if ((l->text() == "Nombre" || l->text() == "Nombre de la granja") &&
				    l->geometry().bottom() <= b->geometry().top() &&
				    l->geometry().left() <= b->geometry().left() + 4) {
					found = true;
				}
			}
			labelAbove = labelAbove && found;
		}
		check(inside, "InputBox entra completo en su columna (sin recorte)");
		check(labelAbove, "etiqueta arriba de cada caja");
	}
	{  // c2-checks
		const QPixmap frame(":/ui/join/input_box.png");
		check(sprites.windowBox().size() == QSize(CHAR_WINDOW_W, CHAR_WINDOW_H),
		      "windowBox 556x520 (278x260 a x2)");
		const QImage cornerExpected =
			frame.copy(0, 0, CHAR_WINDOW_SLICE, CHAR_WINDOW_SLICE)
				.scaled(CHAR_WINDOW_SLICE * CHAR_WINDOW_SCALE,
					CHAR_WINDOW_SLICE * CHAR_WINDOW_SCALE, Qt::IgnoreAspectRatio,
					Qt::FastTransformation)
				.toImage();
		check(sprites.windowBox().toImage().copy(0, 0, CHAR_WINDOW_SLICE * CHAR_WINDOW_SCALE,
							 CHAR_WINDOW_SLICE * CHAR_WINDOW_SCALE) ==
			      cornerExpected,
		      "windowBox: esquina sup-izq intacta");
		auto* win = qobject_cast<QLabel*>(screen.findChild<QWidget*>("characterWindow"));
		check(win != nullptr && win->size() == QSize(CHAR_WINDOW_W, CHAR_WINDOW_H),
		      "characterWindow es QLabel de 556x520");
		QWidget* pc = screen.findChild<QWidget*>("previewColumn");
		QWidget* fc = screen.findChild<QWidget*>("formColumn");
		check(pc && pc->width() == CHAR_PREVIEW_COL_W, "previewColumn 216 px");
		check(fc && fc->width() == CHAR_FORM_COL_W, "formColumn 252 px");
		bool whole = !boxes.isEmpty();
		for (InputBox* b : boxes) {
			whole = whole && b->visibleRegion().boundingRect().width() == b->width();
		}
		check(whole, "InputBox completo (visibleRegion == ancho)");
		bool inside = win != nullptr;
		const int edge = CHAR_WINDOW_SLICE * CHAR_WINDOW_SCALE;
		if (win) {
			const QRect usable = win->rect().adjusted(edge, edge, -edge, -edge);
			for (QWidget* child : win->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
				inside = inside && usable.contains(child->geometry());
			}
		}
		check(inside, "hijos dentro del borde de madera (12 px)");
		check(win && win->layout() && win->layout()->minimumSize().width() <= win->width() &&
			      win->layout()->minimumSize().height() <= win->height(),
		      "contenido no comprimido (minimumSize <= tamano)");
		auto* back = screen.findChild<QWidget*>("backButton");
		auto* status = screen.findChild<QWidget*>("screenStatus");
		bool clear = win && back && status;
		if (clear) {
			const QRect w = win->geometry();
			clear = !w.intersects(back->geometry()) && !w.intersects(status->geometry());
		}
		check(clear, "ventana no pisa Volver ni el estado");
	}
	auto* characterInput = screen.findChild<QLineEdit*>("characterInput");
	auto* gameInput = screen.findChild<QLineEdit*>("gameInput");
	check(characterInput && gameInput &&
		      dynamic_cast<InputBox*>(characterInput->parentWidget()) &&
		      dynamic_cast<InputBox*>(gameInput->parentWidget()),
	      "characterInput/gameInput viven dentro de un InputBox");
	check(characterInput && gameInput && characterInput->placeholderText().isEmpty() &&
		      gameInput->placeholderText().isEmpty(),
	      "sin placeholder (la etiqueta esta arriba)");

	if (gameInput) {
		const QColor normal = gameInput->palette().color(QPalette::Text);
		screen.startJoinGame("Cooperativa");
		settle();
		const QColor muted = gameInput->palette().color(QPalette::Text);
		std::fprintf(stderr, "[debug] normal=%s muted=%s ro=%d\n",
			     normal.name().toUtf8().constData(),
			     muted.name().toUtf8().constData(), gameInput->isReadOnly());
		check(gameInput->isReadOnly() && muted != normal,
		      "gameInput read-only: color atenuado");
		screen.startNewGame(world);
		settle();
		check(!gameInput->isReadOnly() &&
			      gameInput->palette().color(QPalette::Text) == normal,
		      "gameInput vuelve al color normal en NewGame");
	}

	screen.startNewGame(world);
	screen.onEnter();
	settle();
	screen.grab().save(outDir() + "/char_idle.png");
	if (characterInput) characterInput->setText("Granjero");
	if (gameInput) gameInput->setText("La Huerta");
	settle();
	screen.grab().save(outDir() + "/char_text.png");
	screen.startJoinGame("Cooperativa");
	screen.onEnter();
	settle();
	screen.grab().save(outDir() + "/char_join.png");

	if (qEnvironmentVariableIsSet("FONT_PX")) {
		screen.startNewGame(world);
		screen.onExit();  // sin nubes ni caret para que la captura sea comparable
		if (characterInput) characterInput->setText(QString::fromUtf8("Ñoño Árbol"));
		if (gameInput) gameInput->setText("La Huerta Compartida");
		settle();
		screen.grab().save(outDir() + "/char_font_" + qEnvironmentVariable("FONT_PX") + ".png");
	}
	if (qEnvironmentVariableIsSet("BASELINE")) {
		screen.startNewGame(world);
		screen.onExit();  // sin nubes
		if (characterInput) characterInput->setText(QString::fromUtf8("Ñoño Árbol"));
		if (gameInput) gameInput->setText("La Huerta");
		if (QWidget* f = QApplication::focusWidget()) f->clearFocus();  // sin caret
		settle();
		screen.grab().save(qEnvironmentVariable("BASELINE"));
	}

	// ============ C8: comportamiento de la pantalla ============
	{
		HarnessSource catalog2;
		CountingSession session2(30);
		CharacterScreen s2(&session2, &catalog2);
		WorldSummary w2; w2.id = 1; w2.name = "Granja Estandar";
		s2.startNewGame(w2);
		s2.resize(1280, 720);
		s2.show();
		s2.onEnter();
		settle();

		auto* inName = s2.findChild<QLineEdit*>("characterInput");
		auto* inGame = s2.findChild<QLineEdit*>("gameInput");
		auto* create = s2.findChild<QWidget*>("createButton");
		auto* back = s2.findChild<QWidget*>("backButton");
		auto* world = s2.findChild<QWidget*>("worldLabel");
		auto* status = s2.findChild<QLabel*>("screenStatus");
		auto* prevCol = s2.findChild<QWidget*>("previewColumn");

		// --- 1. Ciclos: vuelta en los bordes en las 5 capas ---
		const struct { const char* label; CharacterLayer layer; } cyc[] = {
			{"Piel", CharacterLayer::Skin}, {"Pelo", CharacterLayer::Hair},
			{"Camisa", CharacterLayer::Shirt}, {"Pantalón", CharacterLayer::Pants},
			{"Accesorio", CharacterLayer::Accessory},
		};
		bool wrapOk = true, showOk = true;
		for (const auto& c : cyc) {
			// la fila con esa etiqueta
			QWidget* row = nullptr;
			QString shown;
			for (QWidget* cand : prevCol->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
				for (QLabel* l : cand->findChildren<QLabel*>())
					if (l->text() == QString::fromUtf8(c.label)) {
						row = cand;
						for (QLabel* v : cand->findChildren<QLabel*>())
							if (v->text() != l->text()) shown = v->text();
					}
			if (!row) { std::fprintf(stderr, "[c8] falta la fila %s\n", c.label); wrapOk = false; continue; }
			auto click = [&](const QString& acc) {
				QAbstractButton* b = nullptr;
				for (QAbstractButton* cand : row->findChildren<QAbstractButton*>())
					if (cand->accessibleName() == acc) b = cand;
				if (b) b->click();
				return b != nullptr;
			};
			const int total = catalog2.count(c.layer);
			const QImage before = s2.grab().toImage();
			click(QString::fromUtf8(c.label) + " anterior");  // 0 -> total-1
			if (shown == QString::number(total - 1)) showOk = showOk;
			const QImage afterWrap = s2.grab().toImage();
			QString wrapped;
			for (QLabel* v : row->findChildren<QLabel*>())
				if (v->text() != QString::fromUtf8(c.label)) wrapped = v->text();
			const bool okWrap = wrapped == QString::number(total - 1) && afterWrap != before;
			click(QString::fromUtf8(c.label) + " siguiente");  // total-1 -> 0
			QString back2;
			for (QLabel* v : row->findChildren<QLabel*>())
				if (v->text() != QString::fromUtf8(c.label)) back2 = v->text();
			const bool okBack = back2 == "0";
			std::fprintf(stderr, "[c8] ciclo %-10s total=%2d  anterior->%s  siguiente->%s\n", c.label, total,
				     wrapped.toUtf8().constData(), back2.toUtf8().constData());
			wrapOk = wrapOk && okWrap && okBack;
		}
		check(wrapOk, "ciclos: vuelta en los bordes en las 5 capas (numero y retrato)");

		// --- 2. Sliders: rango y cambio en el retrato ---
		auto* slEyes = s2.findChild<QSlider*>("eyesSlider");
		auto* slHair = s2.findChild<QSlider*>("hairColorSlider");
		auto* slPants = s2.findChild<QSlider*>("pantsColorSlider");
		const bool ranges = slEyes && slHair && slPants &&
			slEyes->maximum() == catalog2.count(CharacterLayer::Eyes) - 1 &&
			slHair->maximum() == catalog2.count(CharacterLayer::HairColor) - 1 &&
			slPants->maximum() == catalog2.count(CharacterLayer::PantsColor) - 1;
		check(ranges, "los 3 sliders cubren todo su catalogo");
		QImage g0, g1, g2;
		if (slHair && slPants) {
			slHair->setValue(0); slPants->setValue(0); settle();
			g0 = s2.grab().toImage();
			slHair->setValue(1); settle();
			g1 = s2.grab().toImage();
			slPants->setValue(0); slHair->setValue(0); slPants->setValue(1); settle();
			g2 = s2.grab().toImage();
			slPants->setValue(0); settle();
			check(g0 != g1 && g0 != g2, "mover el slider de pelo y el de pantalon cambia el retrato");
			check(g0 != g1 && g2 != g1, "los tintes de pelo y pantalon son independientes");
		}

		// --- 3. Validacion NewGame ---
		inName->setText(""); inGame->setText(""); settle();
		const bool emptyDisabled = !create->isEnabled();
		inName->setText("   "); inGame->setText("   "); settle();
		const bool blankDisabled = !create->isEnabled();
		inName->setText(QString::fromUtf8("Ñoño Árbol")); inGame->setText("La Huerta"); settle();
		const bool fullEnabled = create->isEnabled();
		std::fprintf(stderr, "[c8] validacion: vacio=%d espacios=%d completo=%d\n", int(emptyDisabled),
			     int(blankDisabled), int(fullEnabled));
		check(emptyDisabled && blankDisabled && fullEnabled,
		      "Crear: deshabilitado con campos vacios o solo espacios, habilitado al completar");
		inName->setText(QString(CHAR_INPUT_MAX_CHARS + 1, 'a'));
		check(inName->text().size() == CHAR_INPUT_MAX_CHARS,
		      "el caracter de mas no entra (respeta el limite)");

		// --- 4. JoinGame ---
		int confirmed = 0;
		QObject::connect(&s2, &CharacterScreen::characterConfirmed, [&](CharacterSelection) { ++confirmed; });
		s2.startJoinGame("Cooperativa");
		settle();
		inName->setText("Ana"); settle();
		const bool joinOk = inGame->isReadOnly() && inGame->text() == "Cooperativa" &&
				    !world->isVisible() && create->isEnabled();
		const int createsBefore = session2.creates;
		inName->returnPressed(); settle();
		check(joinOk, "unirse: granja read-only, mundo oculto y Crear habilitado con el nombre");
		check(confirmed == 1 && session2.creates == createsBefore,
		      "unirse: confirma sin pedir createGame");

		// --- 5. Ida y vuelta ---
		s2.startNewGame(w2);
		settle();
		auto* worldText = s2.findChild<QLabel*>("worldLabel");
		std::fprintf(stderr, "[c8] vuelta: readOnly=%d mundo_visible=%d texto=\"%s\"\n",
			     int(inGame->isReadOnly()), int(world->isVisible()),
			     worldText ? worldText->text().toUtf8().constData() : "n/d");
		check(!inGame->isReadOnly() && world->isVisible() && worldText &&
			      worldText->text().startsWith("Mundo:"),
		      "volver a nueva partida: granja editable y mundo visible con su texto");

		// --- 6. Estados de red ---
		// Pedido lento (400 ms) para poder mirarlo EN VUELO: settle() espera 80.
		CountingSession slow(400);
		CharacterScreen s4(&slow, &catalog2);
		s4.startNewGame(w2);
		s4.resize(1280, 720);
		s4.show();
		s4.onEnter();
		settle();
		auto* in4 = s4.findChild<QLineEdit*>("characterInput");
		auto* g4 = s4.findChild<QLineEdit*>("gameInput");
		auto* cr4 = s4.findChild<QWidget*>("createButton");
		auto* bk4 = s4.findChild<QWidget*>("backButton");
		auto* st4 = s4.findChild<QLabel*>("screenStatus");
		in4->setText("Ana"); g4->setText("La Huerta"); settle();
		in4->returnPressed();
		in4->returnPressed();  // segundo disparo, con el pedido todavia en vuelo
		settle();
		const bool loading = !cr4->isEnabled() && !bk4->isEnabled() && st4->isVisible() &&
				     st4->text() == "Creando partida...";
		std::fprintf(stderr, "[c8] en vuelo: crea=%d congelado=%d texto=\"%s\"\n", slow.creates,
			     int(loading), st4->text().toUtf8().constData());
		check(loading, "cargando: se congela el formulario y el estado lo dice");
		check(slow.creates == 1, "doble disparo con el pedido en vuelo = un solo createGame");
		// Salir con el pedido en vuelo: la respuesta tardia no entra al juego.
		int lateConfirmed = 0;
		QObject::connect(&s4, &CharacterScreen::characterConfirmed,
				 [&](CharacterSelection) { ++lateConfirmed; });
		s4.onExit();
		{
			QEventLoop loop;
			QTimer::singleShot(500, &loop, &QEventLoop::quit);
			loop.exec();
		}
		check(lateConfirmed == 0, "salir con el pedido en vuelo no emite characterConfirmed");
		// error de nombre tomado
		CountingSession failing(20, ServiceError::GameNameTaken);
		CharacterScreen s3(&failing, &catalog2);
		s3.startNewGame(w2);
		s3.resize(1280, 720);
		s3.show();
		s3.onEnter();
		settle();
		auto* in3 = s3.findChild<QLineEdit*>("characterInput");
		auto* g3 = s3.findChild<QLineEdit*>("gameInput");
		auto* st3 = s3.findChild<QLabel*>("screenStatus");
		auto* cr3 = s3.findChild<QWidget*>("createButton");
		in3->setText("Ana"); g3->setText("La Huerta"); settle();
		in3->returnPressed();
		settle();
		std::fprintf(stderr, "[c8] error: texto=\"%s\" boton_habilitado=%d\n", st3->text().toUtf8().constData(),
			     int(cr3->isEnabled()));
		check(st3->isVisible() && st3->text() == "Ya existe una partida con ese nombre." && cr3->isEnabled(),
		      "error de nombre tomado: muestra el mensaje y rehabilita el formulario");

		// --- 7. Volver en modo unirse ---
		int nav = -1;
		QObject::connect(&s2, &CharacterScreen::requestNavigate, [&](ScreenId id) { nav = int(id); });
		s2.startJoinGame("Cooperativa");
		settle();
		if (auto* b = s2.findChild<QAbstractButton*>("backButton")) b->click();
		settle();
		check(nav == int(ScreenId::JoinCode), "Volver en modo unirse navega al codigo");
	}

	if (qEnvironmentVariableIsSet("C8_SHOTS")) {
		// Todo con la API publica: widgets y una sesion que falla cuando hace falta.
		const QString dir = qEnvironmentVariable("C8_SHOTS");
		HarnessSource cats;
		CountingSession ok(20);
		CharacterScreen cs(&ok, &cats);
		WorldSummary ws;
		ws.id = 1;
		ws.name = QString::fromUtf8("Granja Estándar").toStdString();
		auto* in = cs.findChild<QLineEdit*>("characterInput");
		auto* gm = cs.findChild<QLineEdit*>("gameInput");
		auto* col = cs.findChild<QWidget*>("previewColumn");
		cs.resize(1280, 720);
		cs.show();
		cs.onEnter();
		cs.startNewGame(ws);
		cs.onExit();  // sin nubes ni caret
		settle();
		cs.grab().save(dir + "/c8_newgame_vacio.png");

		in->setText(QString::fromUtf8("Ñoño Árbol"));
		gm->setText("La Huerta Compartida");
		auto step = [&](const char* label, int times) {
			for (QWidget* row : col->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
				bool mine = false;
				for (QLabel* l : row->findChildren<QLabel*>())
					mine = mine || l->text() == QString::fromUtf8(label);
				if (!mine) continue;
				for (QAbstractButton* b : row->findChildren<QAbstractButton*>())
					if (b->accessibleName() == QString::fromUtf8(label) + " siguiente")
						for (int i = 0; i < times; ++i) b->click();
			}
		};
		step("Pelo", 3);
		step("Camisa", 4);
		step("Accesorio", 2);
		if (auto* e = cs.findChild<QSlider*>("eyesSlider")) e->setValue(2);
		if (auto* h = cs.findChild<QSlider*>("hairColorSlider")) h->setValue(6);
		if (auto* p = cs.findChild<QSlider*>("pantsColorSlider")) p->setValue(3);
		settle();
		cs.grab().save(dir + "/c8_newgame_completo.png");

		cs.startJoinGame("Cooperativa de la Costa");
		cs.onExit();
		settle();
		cs.grab().save(dir + "/c8_joingame.png");

		// Error: una sesion que rechaza el nombre ya usado.
		CountingSession failing(20, ServiceError::GameNameTaken);
		CharacterScreen es(&failing, &cats);
		es.resize(1280, 720);
		es.show();
		es.onEnter();
		es.startNewGame(ws);
		auto* ein = es.findChild<QLineEdit*>("characterInput");
		auto* egm = es.findChild<QLineEdit*>("gameInput");
		ein->setText("Ana");
		egm->setText("La Huerta");
		settle();
		ein->returnPressed();
		settle();
		es.onExit();
		settle();
		es.grab().save(dir + "/c8_error.png");
		std::printf("capturas C8 en %s\n", dir.toUtf8().constData());
	}

	{  // musica de la pantalla
		RecordingMusic rec;
		HarnessSource cats3;
		FakeSessionService sess3;
		CharacterScreen ms(&sess3, &cats3, nullptr, &rec);
		ms.startNewGame(WorldSummary{});
		ms.resize(1280, 720);
		ms.show();
		check(rec.calls == 0, "abrir la pantalla no pide musica hasta onEnter");
		ms.onEnter();
		settle();
		std::fprintf(stderr, "[mus] pista pedida: %s (calls=%d)\n",
			     rec.last == core::MusicTrack::CharacterSelect ? "CharacterSelect" : "otra", rec.calls);
		check(rec.calls == 1 && rec.last == core::MusicTrack::CharacterSelect,
		      "al entrar pide la musica propia de la pantalla (character_select)");
		ms.onExit();
		settle();
		std::fprintf(stderr, "[mus] al salir pide: %s (calls=%d)\n",
			     rec.last == core::MusicTrack::MenuTheme ? "MenuTheme" : "otra", rec.calls);
		check(rec.calls == 2 && rec.last == core::MusicTrack::MenuTheme && rec.stops == 0,
		      "al salir vuelve la musica del lobby (sin cortar el audio)");
		// sin musica (nullptr) no rompe
		HarnessSource cats4;
		FakeSessionService sess4;
		CharacterScreen bare(&sess4, &cats4);
		bare.startNewGame(WorldSummary{});
		bare.show();
		bare.onEnter();
		settle();
		check(true, "sin MusicControl la pantalla funciona igual");
	}

	std::printf("%s (%d fallos)\n", failures == 0 ? "OK" : "FAIL", failures);
	return failures == 0 ? 0 : 1;
}
