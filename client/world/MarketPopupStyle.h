#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/PixelBox.h"
#include "world/SpriteVisual.h"

namespace world {

struct MarketItem {
	std::string contentId;
	std::string label;
	// Sin precio = solo se muestra el nombre.
	std::optional<int> price;
};

struct MarketSection {
	std::string title;
	std::vector<MarketItem> items;
};

// Estilo y geometría del popup (datos puros, sin SDL). Las cajas se
// calculan dentro del panel; el painter solo pinta.
struct MarketPopupStyle {
	// Columnas de slots del panel; las filas salen del contenido de cada
	// oferta (contentHeight).
	int panelColumns = 0;
	int inset = 0;
	int columns = 1;
	int titleHeight = 0;
	int sectionGap = 0;
	int rowHeight = 0;
	int priceHeight = 0;
	int iconSize = 0;
	int iconGap = 0;
	// Baja el nombre respecto del tope del ícono (px).
	int labelOffsetY = 0;
	// Moneda al costado del precio y separación con el número.
	SpriteVisual coin;
	int coinGap = 0;
	int titleSize = 0;
	int textSize = 0;
	Color titleColor{0, 0, 0, 255};
	Color textColor{0, 0, 0, 255};
	Color priceColor{0, 0, 0, 255};

	// Alto que ocupa el contenido de `sections` (márgenes incluidos): lo
	// que el panel debe poder contener.
	int contentHeight(const std::vector<MarketSection>& sections) const;

	PixelBox titleBox(const PixelBox& panel,
			  const std::vector<MarketSection>& sections,
			  std::size_t section) const;
	PixelBox cellBox(const PixelBox& panel,
			 const std::vector<MarketSection>& sections,
			 std::size_t section, std::size_t item) const;
	PixelBox iconBox(const PixelBox& cell) const;
	PixelBox labelBox(const PixelBox& cell, bool withPrice) const;
	PixelBox coinBox(const PixelBox& cell) const;
	PixelBox priceBox(const PixelBox& cell) const;

private:
	int sectionTop(const PixelBox& panel,
		       const std::vector<MarketSection>& sections,
		       std::size_t section) const;
	int rowsOf(const MarketSection& section) const;
};

}
