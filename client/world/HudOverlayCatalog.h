#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/AnchorLayout.h"
#include "world/Color.h"
#include "world/SpriteVisual.h"

namespace world {

// Cómo se mezcla un velo de color con lo que ya está dibujado: Alpha lo
// pinta encima con transparencia; Multiply multiplica cada píxel por el
// color (el negro sigue negro, solo se tiñe lo que tiene luz).
enum class BlendMode {
	Alpha,
	Multiply,
};

// Un overlay de HUD: un sprite de una hoja dibujado en una posición fija
// de la pantalla (anclado a un borde/centro, sin cámara). Es DATO de
// presentación (convención §5): qué hoja, qué región, dónde y a qué
// escala. Es el caso general de cosas fijas como el HUD de tiempo: sumar
// otra es sumar una entrada al TOML, sin tocar el dibujo.
struct HudOverlay {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;
	SheetRegion region;
	Anchor anchor = Anchor::TopRight;
	int marginX = 0;
	int marginY = 0;
	int scale = 1;
	// Opacidad máxima del overlay (0..255, key opcional `alpha`).
	int alpha = 255;
	// Color plano (key opcional `color = [r, g, b]`). Si está, el overlay
	// es un velo que cubre toda la ventana y NO usa hoja ni anclaje.
	std::optional<Color> color = std::nullopt;
	// Cómo se mezcla el velo de color (key opcional `blend`).
	BlendMode blend = BlendMode::Alpha;
};

// Cómo se interpreta el valor crudo de un {token} de una etiqueta. TODO es
// dato (convención §5): el servidor manda números y el TOML decide cómo
// verlos. Con todo en su default, el token escribe el número tal cual.
//   valor = (crudo / divisor), y si modulo > 0, % modulo
//   texto = names[valor] si hay `names`; si no, el número con `pad` dígitos.
struct HudTokenFormat {
	// Clave de los valores de la que sale el crudo (vacío = el nombre
	// del propio token). Permite que dos tokens salgan del mismo valor
	// (la hora en 12 h y am/pm).
	std::string source;
	int divisor = 1;
	int modulo = 0;
	int pad = 0;
	std::vector<std::string> names;
};

// Una etiqueta de texto del HUD: una plantilla con {tokens} dibujada en
// una caja anclada a la ventana (sin cámara). Qué dice cada token lo
// manda la fuente de estado; cómo se muestra lo dice `tokens`.
struct HudLabel {
	std::string text;
	Anchor anchor = Anchor::TopRight;
	int marginX = 0;
	int marginY = 0;
	int boxWidth = 0;
	int boxHeight = 0;
	int fontSize = 0;
	Color color = Color{0, 0, 0, 255};
	std::map<std::string, HudTokenFormat> tokens;
};

// Catálogo de overlays de HUD, en el orden en que se dibujan. Datos puros
// sin SDL: la capa sdl/ los pinta.
class HudOverlayCatalog {
public:
	const std::vector<HudOverlay>& overlays() const;

	void add(const HudOverlay& overlay);

	const std::vector<HudLabel>& labels() const;

	void addLabel(const HudLabel& label);

	// El texto final de una etiqueta: su plantilla con cada {token}
	// reemplazado por el valor crudo que mandó la fuente de estado,
	// interpretado según el formato del token (ver HudTokenFormat). Un
	// token sin valor, o con un índice fuera de `names`, queda literal
	// (a la vista, como el popup).
	std::string textFor(const HudLabel& label,
			    const std::map<std::string, int>& values) const;

	// Opacidad final (0..255) del overlay para una intensidad de 0.0 a
	// 1.0: su alpha máximo escalado. Con intensidad 1.0 es su alpha.
	int alphaFor(const HudOverlay& overlay, double intensity) const;

	// Color multiplicador del velo Multiply para una intensidad: va de
	// blanco (no cambia nada) con intensidad 0.0 al color del overlay con
	// 1.0. Su `alpha` actúa como fuerza máxima del efecto.
	Color multiplyColorFor(const HudOverlay& overlay,
			       double intensity) const;

private:
	// El texto de UN token, o nada si no hay valor / índice válido.
	std::optional<std::string> tokenText(
		const HudLabel& label, const std::string& token,
		const std::map<std::string, int>& values) const;

	std::vector<HudOverlay> overlays_;
	std::vector<HudLabel> labels_;
};

}
