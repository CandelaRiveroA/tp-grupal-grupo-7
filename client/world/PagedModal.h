#pragma once

#include <string>

#include "world/Color.h"
#include "world/PixelBox.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Especificación visual común a cualquier modal paginado con pergamino,
// marco de madera y flechas laterales (Mapa del Mundo, Centro Comunitario).
// Datos puros sin SDL; se construye desde el TOML ([panel], [arrows], [colors]).
struct PagedModalSpec {
	PagedModalSpec();
	explicit PagedModalSpec(const tomlcfg::TomlDocument& doc);

	double widthFraction = 0.0;
	double heightFraction = 0.0;
	int minWidth = 0;
	int minHeight = 0;
	int maxWidth = 0;
	int maxHeight = 0;
	int framePad = 0;
	int bgInset = 0;

	int arrowWidth = 0;
	int arrowHeight = 0;
	int arrowMargin = 0;
	int arrowGap = 0;
	std::string arrowLeftSheet;
	std::string arrowRightSheet;
	std::string arrowLeftFallback;
	std::string arrowRightFallback;
	int arrowFontSize = 0;

	Color backdropColor{};
	Color parchmentColor{};
	Color woodBorderColor{};
	Color inkColor{};
};

// Geometría pura del contenedor paginado: panel centrado, pergamino interior,
// flechas izquierda/derecha y área útil de contenido entre ambas flechas.
class PagedModalLayout {
public:
	PagedModalLayout(int screenW, int screenH, const PagedModalSpec& spec,
			 int contentTopOffset, int contentBottomMargin);

	const PixelBox& panel() const;
	PixelBox backgroundBox() const;
	const PixelBox& leftArrowRect() const;
	const PixelBox& rightArrowRect() const;
	const PixelBox& innerContentRect() const;

	bool hitsLeftArrow(int x, int y) const;
	bool hitsRightArrow(int x, int y) const;
	bool hitsAnyArrow(int x, int y) const;

private:
	PixelBox panel_{};
	PixelBox leftArrowRect_{};
	PixelBox rightArrowRect_{};
	PixelBox innerContentRect_{};
	int bgInset_ = 0;
};

}  // namespace world
