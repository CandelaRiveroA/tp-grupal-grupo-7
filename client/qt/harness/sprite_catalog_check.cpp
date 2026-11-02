#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QMap>
#include <QPixmap>
#include <QPoint>
#include <QVector>
#include <QSet>

#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <QDir>

#include <string>
#include <utility>

#include "qt/services/CharacterSpriteComposer.h"
#include "qt/theme/CharacterSheetRects.h"
#include "qt/util/CharacterLayer.h"
#include "qt/widgets/CharacterPreview.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CharacterPaletteLoader.h"

namespace {
QString outDir() {
	const QString dir = qEnvironmentVariable("HARNESS_OUT", QDir::currentPath() + "/harness-out");
	QDir().mkpath(dir);
	return dir;
}
int failures = 0;
void check(bool ok, const char* name) {
	std::printf("%-70s %s\n", name, ok ? "PASS" : "FAIL");
	if (!ok) ++failures;
}
// La raiz del repo la define CMake (TALLER_SOURCE_DIR).
const std::string kRepo = TALLER_SOURCE_DIR;
QImage argb(const QPixmap& p) { return p.toImage().convertToFormat(QImage::Format_ARGB32); }
}  // namespace

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	const world::CharacterCatalog characters = world::CharacterCatalogLoader().loadFromFile(
		kRepo + "/client/sdl/assets/characters.toml");
	QFile pal(":/character/palettes.toml");
	check(pal.open(QIODevice::ReadOnly), "palettes.toml esta en el qrc");
	const world::CharacterPalettes palettes =
		world::CharacterPaletteLoader().loadFromString(pal.readAll().toStdString());
	CharacterSpriteComposer cat(characters, palettes);

	check(cat.canvasSize() == QSize(32, 48), "canvas 32x48");
	check(cat.count(CharacterLayer::Skin) == 24 && cat.count(CharacterLayer::Eyes) == 12 &&
		      cat.count(CharacterLayer::Hair) == 32 && cat.count(CharacterLayer::HairColor) == 16 &&
		      cat.count(CharacterLayer::Shirt) == 48 && cat.count(CharacterLayer::Pants) == 7 &&
		      cat.count(CharacterLayer::Accessory) == 20,
	      "conteos 24/12/32/16/48/7/20 (subconjunto elegido)");

	bool allOk = true;
	for (int l = 0; l < static_cast<int>(CharacterLayer::Count); ++l) {
		const auto layer = static_cast<CharacterLayer>(l);
		for (int i = 0; i < cat.count(layer); ++i) {
			const QPixmap p = cat.pixmap(layer, static_cast<std::uint8_t>(i));
			allOk = allOk && !p.isNull() && p.size() == QSize(32, 48);
		}
	}
	check(allOk, "todos los pixmaps son 32x48 y no nulos");
	check(cat.pixmap(CharacterLayer::Skin, 200).size() == QSize(32, 48),
	      "indice fuera de rango no rompe");

	// --- Equivalencia con SDL: mismo PNG, misma pose (frente / idle) ---
	QImage sdlBase(QString::fromStdString(kRepo + "/client/sdl/assets/Players/main/Character_Base.png"));
	QImage qtBase(QString::fromStdString(kRepo + "/client/qt/resources/character/Character_Base.png"));
	check(!sdlBase.isNull() && sdlBase == qtBase, "Character_Base.png de Qt y de SDL son identicos");
	QImage expected(32, 48, QImage::Format_ARGB32);
	expected.fill(Qt::transparent);
	{
		QPainter p(&expected);
		p.drawImage(QPoint(SHEET_BODY_X, SHEET_BODY_Y), sdlBase, QRect(0, 0, 16, 32));     // cuerpo  [0,0,1,2]
		p.drawImage(QPoint(SHEET_BODY_X, SHEET_BODY_Y + 16), sdlBase, QRect(96, 16, 16, 16));  // brazos [6,1,1,1], anclados abajo
	}
	check(argb(cat.pixmap(CharacterLayer::Skin, 0)) == expected,
	      "Skin 0 == cuerpo+brazos de frente/idle del PNG de SDL (pixel a pixel)");

	// --- Skin N: solo cambian los 3 colores testigo ---
	QImage skins(QString::fromStdString(kRepo + "/client/qt/resources/character/skinColors..png"));
	const QRgb w0 = skins.pixel(0, 0), w1 = skins.pixel(1, 0), w2 = skins.pixel(2, 0);
	const QImage s0 = argb(cat.pixmap(CharacterLayer::Skin, 0));
	const QImage s5 = argb(cat.pixmap(CharacterLayer::Skin, 5));
	bool skinOk = true, changed = false;
	for (int y = 0; y < 48; ++y)
		for (int x = 0; x < 32; ++x) {
			const QRgb a = s0.pixel(x, y), b = s5.pixel(x, y);
			QRgb want = a;
			if (qAlpha(a) == 255) {
				if ((a & 0xFFFFFF) == (w0 & 0xFFFFFF)) want = skins.pixel(0, 5);
				else if ((a & 0xFFFFFF) == (w1 & 0xFFFFFF)) want = skins.pixel(1, 5);
				else if ((a & 0xFFFFFF) == (w2 & 0xFFFFFF)) want = skins.pixel(2, 5);
			}
			skinOk = skinOk && (b == want);
			changed = changed || (a != b);
		}
	check(skinOk && changed, "Skin 5: solo cambian los 3 colores testigo (cuerpo y brazos)");

	// --- Ojos ---
	const QImage e0 = argb(cat.pixmap(CharacterLayer::Eyes, 0));
	const QImage e2 = argb(cat.pixmap(CharacterLayer::Eyes, 2));
	int solid0 = 0, solid2 = 0;
	for (int y = 0; y < 48; ++y)
		for (int x = 0; x < 32; ++x) {
			solid0 += qAlpha(e0.pixel(x, y)) == 255;
			solid2 += qAlpha(e2.pixel(x, y)) == 255;
		}
	check(solid0 == 4 && solid2 == 4, "Eyes: solo las 4 pupilas del cuerpo");
	check(QColor(e0.pixel(SHEET_BODY_X + 6, SHEET_BODY_Y + 11)) == QColor(104, 43, 15) &&
		      QColor(e2.pixel(SHEET_BODY_X + 9, SHEET_BODY_Y + 12)) == QColor(34, 80, 190),
	      "Eyes: pupila 0 marron (como el PNG) y pupila 2 azul (paleta TOML)");

	// --- Pelo, camisa, pantalon, accesorio ---
	QImage hairs(QString::fromStdString(kRepo + "/client/qt/resources/character/StardewValley_Character_Hairstyles.png"));
	QImage h17(32, 48, QImage::Format_ARGB32);
	h17.fill(Qt::transparent);
	{
		QPainter p(&h17);
		p.drawImage(QPoint(SHEET_BODY_X, SHEET_BODY_Y), hairs, QRect(16, 96, 16, 32));  // indice 17 = columna 1, fila 1
	}
	{
		// QPixmap guarda los colores premultiplicados: en los pixeles
		// semitransparentes el color puede variar 1 por redondeo. El alfa y
		// los pixeles opacos tienen que ser exactos.
		const QImage got = argb(cat.pixmap(CharacterLayer::Hair, 17));
		bool same = true;
		for (int y = 0; y < 48 && same; ++y)
			for (int x = 0; x < 32 && same; ++x) {
				const QRgb g = got.pixel(x, y), w = h17.pixel(x, y);
				if (qAlpha(g) != qAlpha(w)) { same = false; break; }
				if (qAlpha(w) == 255) { same = (g == w); continue; }
				same = std::abs(qRed(g) - qRed(w)) <= 1 && std::abs(qGreen(g) - qGreen(w)) <= 1 &&
				       std::abs(qBlue(g) - qBlue(w)) <= 1;
			}
		check(same, "Hair 17 == celda (1,1) de la hoja (16 columnas, paso 96)");
	}
	const QImage hc = argb(cat.pixmap(CharacterLayer::HairColor, 3));
	check(QColor(hc.pixel(0, 0)) == QColor(140, 90, 50) && QColor(hc.pixel(31, 47)) == QColor(140, 90, 50),
	      "HairColor 3 = color plano de la paleta TOML");
	// --- Camisa: torso 8x8 + mangas (los 14 px marcadores de los brazos) ---
	QSet<int> sleevePts;  // clave = y * 32 + x
	QMap<int, int> sleeveTone;  // 0 oscuro, 1 medio, 2 claro
	for (int y = 0; y < 16; ++y)
		for (int x = 0; x < 16; ++x) {
			const QRgb c = sdlBase.pixel(96 + x, 16 + y);
			if (qAlpha(c) != 255) continue;
			const QRgb rgb = c & 0xFFFFFF;
			int tone = -1;
			if (rgb == 0x4a0c06) tone = 0;
			else if (rgb == 0x701718) tone = 1;
			else if (rgb == 0x8e1f0c) tone = 2;
			if (tone >= 0) {
				const int at = (SHEET_BODY_Y + 16 + y) * 32 + (SHEET_BODY_X + x);  // brazos anclados abajo del cuerpo
				sleevePts.insert(at);
				sleeveTone.insert(at, tone);
			}
		}
	check(sleevePts.size() == 14, "los brazos de SDL tienen 14 px de manga (marcadores)");
	const QImage sh = argb(cat.pixmap(CharacterLayer::Shirt, 0));
	bool shirtOk = true, shirtAny = false;
	for (int y = 0; y < 48; ++y)
		for (int x = 0; x < 32; ++x)
			if (qAlpha(sh.pixel(x, y))) {
				shirtAny = true;
				shirtOk = shirtOk && (QRect(SHEET_BODY_X + SHEET_SHIRT_X, SHEET_BODY_Y + SHEET_SHIRT_Y, 8, 8).contains(x, y) || sleevePts.contains(y * 32 + x));
			}
	check(shirtAny && shirtOk, "Shirt 0 ocupa solo el torso 8x8 (12,27) y los px de manga");
	auto sleeveColors = [&](int idx) {
		const QImage im = argb(cat.pixmap(CharacterLayer::Shirt, static_cast<std::uint8_t>(idx)));
		QVector<QColor> tones(3);
		bool consistent = true;
		for (auto it = sleeveTone.constBegin(); it != sleeveTone.constEnd(); ++it) {
			const QRgb c = im.pixel(it.key() % 32, it.key() / 32);
			if (qAlpha(c) != 255) { consistent = false; continue; }
			if (tones[it.value()].isValid() && tones[it.value()].rgb() != c) consistent = false;
			tones[it.value()] = QColor(c);
		}
		return std::make_pair(tones, consistent);
	};
	const auto s1 = sleeveColors(1);
	const auto s2 = sleeveColors(2);
	check(s1.second && s1.first[0] == QColor(0x64, 0x27, 0x00) && s1.first[1] == QColor(0xa1, 0x58, 0x00) &&
		      s1.first[2] == QColor(0xcf, 0x79, 0x00),
	      "Shirt 1 (naranja): manga = sus 3 tonos principales (642700, a15800, cf7900)");
	check(s2.second && s2.first[0] == QColor(0x1a, 0x33, 0x22) && s2.first[1] == QColor(0x3e, 0x6a, 0x4e) &&
		      s2.first[2] == QColor(0x9b, 0xe0, 0xb4),
	      "Shirt 2 (verde): manga cambia con la camisa (1a3322, 3e6a4e, 9be0b4)");
	check(s1.first != s2.first, "la manga de dos camisas distintas no es igual");
	{  // sleeve-all
		// Camisa 0 = identidad: sus 3 tonos son los marcadores de los brazos.
		const QImage sh0 = argb(cat.pixmap(CharacterLayer::Shirt, 0));
		const QImage skin0 = argb(cat.pixmap(CharacterLayer::Skin, 0));
		bool identity = !sleeveTone.isEmpty();
		for (auto it = sleeveTone.constBegin(); it != sleeveTone.constEnd(); ++it)
			identity = identity && sh0.pixel(it.key() % 32, it.key() / 32) == skin0.pixel(it.key() % 32, it.key() / 32);
		check(identity, "Shirt 0: la manga queda igual a los brazos originales (identidad)");
		int covered = 0;
		for (int idx = 0; idx < 48; ++idx) {
			const QImage im = argb(cat.pixmap(CharacterLayer::Shirt, static_cast<std::uint8_t>(idx)));
			bool all = true;
			for (auto it = sleeveTone.constBegin(); it != sleeveTone.constEnd(); ++it)
				all = all && qAlpha(im.pixel(it.key() % 32, it.key() / 32)) == 255;
			covered += all;
		}
		std::fprintf(stderr, "[m] camisas con los 14 px de manga cubiertos: %d de 48\n", covered);
		check(covered == 48, "las 48 camisas pintan los 14 px de manga");
	}
	// La piel no se toca: los marcadores quedan debajo, tapados por la manga.
	check(argb(cat.pixmap(CharacterLayer::Skin, 0)) == expected, "Skin sigue igual al PNG de SDL");
	const QImage acc0 = argb(cat.pixmap(CharacterLayer::Accessory, 0));
	bool accBlank = true;
	for (int y = 0; y < 48; ++y)
		for (int x = 0; x < 32; ++x) accBlank = accBlank && qAlpha(acc0.pixel(x, y)) == 0;
	check(accBlank, "Accessory 0 = sin accesorio (transparente)");
	check(argb(cat.pixmap(CharacterLayer::Accessory, 1)) != acc0, "Accessory 1 dibuja algo");
	check(argb(cat.pixmap(CharacterLayer::Pants, 0)) != acc0, "Pants 0 dibuja algo");

	{  // pants-checks
		check(cat.count(CharacterLayer::Pants) == 7, "Pants: 7 pantalones teñibles (sin los de color propio 5, 6, 7)");
		check(cat.count(CharacterLayer::PantsColor) == 16, "PantsColor: 16 colores de la paleta TOML");
		const QImage pc2 = argb(cat.pixmap(CharacterLayer::PantsColor, 3));
		check(QColor(pc2.pixel(0, 0)) == QColor(40, 70, 150), "PantsColor 3 = color plano de la paleta TOML");
		// ningun pantalon de la lista tiene color propio: todos casi grises
		bool allGrey = true;
		for (int i = 0; i < cat.count(CharacterLayer::Pants); ++i) {
			const QImage im = argb(cat.pixmap(CharacterLayer::Pants, static_cast<std::uint8_t>(i)));
			int px = 0, grey = 0;
			for (int y = 0; y < 48; ++y)
				for (int x = 0; x < 32; ++x) {
					const QRgb c = im.pixel(x, y);
					if (qAlpha(c) == 0) continue;
					++px;
					grey += (std::max({qRed(c), qGreen(c), qBlue(c)}) - std::min({qRed(c), qGreen(c), qBlue(c)})) < 18;
				}
			allGrey = allGrey && px > 0 && grey * 100 / px >= 70;
		}
		check(allGrey, "todos los pantalones de la lista son >= 70% grises (se tiñen bien)");
		// el compuesto: el pantalon teñido difiere del sin teñir y conserva varios tonos
		CharacterPreview pv(&cat);
		pv.setFixedSize(128, 192);
		pv.show();
		CharacterAppearance ap;
		ap.shirt = 1; ap.pants = 0; ap.pantsColor = 0;
		pv.setAppearance(ap);
		const QImage g0 = pv.grab().toImage();
		ap.pantsColor = 7;
		pv.setAppearance(ap);
		const QImage g1 = pv.grab().toImage();
		check(g0 != g1, "cambiar PantsColor cambia el retrato");
		QSet<QRgb> tones;
		for (int y = 0; y < 192; ++y)
			for (int x = 0; x < 128; ++x) {
				// zona del pantalon: filas 22..26 del cuerpo
				if (y >= (SHEET_BODY_Y + 22) * 4 && y < (SHEET_BODY_Y + 27) * 4) tones.insert(g1.pixel(x, y));
			}
		check(tones.size() >= 5, "pantalon teñido conserva varios tonos (sombreado)");
	}
	// --- Orden de capas: el pelo va arriba de todo ---
	check(static_cast<int>(CharacterLayer::Hair) > static_cast<int>(CharacterLayer::Accessory) &&
		      static_cast<int>(CharacterLayer::Accessory) > static_cast<int>(CharacterLayer::Shirt) &&
		      static_cast<int>(CharacterLayer::Shirt) > static_cast<int>(CharacterLayer::Pants) &&
		      static_cast<int>(CharacterLayer::Pants) > static_cast<int>(CharacterLayer::Skin),
	      "orden de capas: Skin < Pants < Shirt < Accessory < Hair");

	{  // tint-check: el sombreado del peinado sobrevive al tinte
		CharacterPreview only(&cat);
		only.setFixedSize(128, 192);
		only.show();
		CharacterAppearance ap;
		ap.skin = 0; ap.hair = 0; ap.hairColor = 3;  // castano (140,90,50)
		only.setAppearance(ap);
		const QImage shot = only.grab().toImage();
		QSet<QRgb> hairColors;
		for (int y = 0; y < 30; ++y)  // franja de la cabeza (arriba del ojo), escala x4
			for (int x = 0; x < 128; ++x) hairColors.insert(shot.pixel(x, SHEET_BODY_Y * 4 + y));
		check(hairColors.size() >= 4, "pelo teñido conserva varios tonos (sombreado)");
	}
	{  // feet-check: los pies sobre el centro de la elipse oscura del fondo diurno
		QImage bg(":/ui/character/backgrounds.png");
		bg = bg.copy(0, 0, 128, 192).convertToFormat(QImage::Format_ARGB32);
		// elipse = pixeles #014719 del fondo
		int ex0 = 999, ex1 = -1, ey0 = 999, ey1 = -1;
		for (int y = 0; y < 192; ++y)
			for (int x = 0; x < 128; ++x)
				if ((bg.pixel(x, y) & 0xFFFFFF) == 0x014719) {
					ex0 = std::min(ex0, x); ex1 = std::max(ex1, x);
					ey0 = std::min(ey0, y); ey1 = std::max(ey1, y);
				}
		const double ecx = (ex0 + ex1) / 2.0, ecy = (ey0 + ey1) / 2.0;
		std::fprintf(stderr, "[m] elipse x %d..%d y %d..%d centro (%.1f, %.1f)\n", ex0, ex1, ey0, ey1, ecx, ecy);
		// zapatos del compuesto completo (todas las capas, tal como las dibuja el preview)
		CharacterPreview feet(&cat);
		feet.setFixedSize(128, 192);
		feet.setBackground(QPixmap::fromImage(bg));
		feet.show();
		CharacterAppearance ap;
		ap.pants = 0; ap.shirt = 1; ap.hair = 0; ap.hairColor = 3;
		feet.setAppearance(ap);
		const QImage shot = feet.grab().toImage().convertToFormat(QImage::Format_ARGB32);
		// zapatos = colores de bota del cuerpo base (3d1123, 5b1f24, 77291a, ad471b), x4
		const QSet<QRgb> shoe = {0x3d1123, 0x5b1f24, 0x77291a, 0xad471b};
		int fy0 = 999, fy1 = -1, fx0 = 999, fx1 = -1;
		for (int y = 0; y < 192; ++y)
			for (int x = 0; x < 128; ++x)
				if (shoe.contains(shot.pixel(x, y) & 0xFFFFFF) && y > 100) {
					fy0 = std::min(fy0, y); fy1 = std::max(fy1, y);
					fx0 = std::min(fx0, x); fx1 = std::max(fx1, x);
				}
		const double fcy = (fy0 + fy1) / 2.0, fcx = (fx0 + fx1) / 2.0;
		std::fprintf(stderr, "[m] zapatos x %d..%d y %d..%d centro (%.1f, %.1f)\n", fx0, fx1, fy0, fy1, fcx, fcy);
		check(std::abs(fcx - ecx) <= 2.0, "pies centrados en X con la elipse oscura (<= 2 px)");
		check(std::abs(fcy - ecy) <= 2.0, "pies centrados en Y con la elipse oscura (<= 2 px)");
		check(fy1 <= ey1 + 1 && fx0 >= ex0 && fx1 <= ex1, "los zapatos caen dentro de la elipse");
		// nada del personaje se corta arriba: el tope del lienzo (y=0) queda vacio en todas las capas
		bool topClear = true;
		for (int l = 0; l < static_cast<int>(CharacterLayer::Count); ++l) {
			// HairColor y PantsColor son color plano de todo el lienzo: no son sprites
			if (static_cast<CharacterLayer>(l) == CharacterLayer::HairColor ||
			    static_cast<CharacterLayer>(l) == CharacterLayer::PantsColor) continue;
			const auto layer = static_cast<CharacterLayer>(l);
			for (int i = 0; i < cat.count(layer); ++i) {
				const QImage im = argb(cat.pixmap(layer, static_cast<std::uint8_t>(i)));
				for (int x = 0; x < 32; ++x) topClear = topClear && qAlpha(im.pixel(x, 0)) == 0;
			}
		}
		check(topClear, "ninguna capa toca la fila 0 del lienzo (nada recortado arriba)");
	}
	// --- Preview a 128x192 ---
	CharacterPreview preview(&cat);
	preview.setFixedSize(128, 192);
	preview.show();
	CharacterAppearance a;
	preview.setAppearance(a);
	const QImage g0 = preview.grab().toImage();
	a.hair = 17;
	preview.setAppearance(a);
	const QImage g1 = preview.grab().toImage();
	a.hair = 17; a.hairColor = 9;
	preview.setAppearance(a);
	const QImage g2 = preview.grab().toImage();
	check(g0 != g1 && g1 != g2, "CharacterPreview cambia al variar peinado y color de pelo");
	g2.save(outDir() + "/preview.png");

	std::printf("%s (%d fallos)\n", failures == 0 ? "OK" : "FAIL", failures);
	return failures == 0 ? 0 : 1;
}
