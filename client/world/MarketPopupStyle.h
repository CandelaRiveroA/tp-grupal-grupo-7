#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/Color.h"
#include "world/CropQuality.h"
#include "world/Grid.h"
#include "world/PixelBox.h"
#include "world/SpriteVisual.h"

namespace world {

// Una línea de calidad de un producto (ya resuelta para dibujar): su
// nombre, la estrella que la acompaña (sin estrella = ninguna) y su monto.
struct MarketTier {
	CropQuality quality = CropQuality::Base;
	std::string label;
	std::optional<SheetCell> star;
	// Sin precio = solo se muestra el nombre.
	std::optional<int> price;
};

struct MarketItem {
	std::string contentId;
	std::string label;
	// Sin precio = solo se muestra el nombre.
	std::optional<int> price;
	// Variantes por calidad, de menor a mayor. Con tiers, el ítem se
	// dibuja como encabezado (ícono + nombre) y una línea por calidad
	// debajo; el `price` propio no se usa.
	std::vector<MarketTier> tiers;
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
	// Ítems con calidades: alto del encabezado (ícono + nombre), alto de
	// cada línea de calidad, desde dónde (px desde el borde izquierdo de
	// la celda) arranca la columna del precio y separación entre la
	// estrella y el nombre. `star` es la hoja de estrellas (la celda de
	// cada calidad viaja en MarketTier).
	int headerHeight = 0;
	int tierHeight = 0;
	int tierPriceLeft = 0;
	int starGap = 0;
	SpriteVisual star;
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

	// Encabezado de un ítem con calidades (ocupa lo mismo que el ícono y
	// el nombre de un ítem común: se dibuja con iconBox/labelBox).
	PixelBox headerBox(const PixelBox& cell) const;
	// Línea `index` de calidad dentro de la celda, y sus partes.
	PixelBox tierLineBox(const PixelBox& cell, std::size_t index) const;
	PixelBox starBox(const PixelBox& line) const;
	PixelBox tierLabelBox(const PixelBox& line) const;
	PixelBox tierCoinBox(const PixelBox& line) const;
	PixelBox tierPriceBox(const PixelBox& line) const;

private:
	int sectionTop(const PixelBox& panel,
		       const std::vector<MarketSection>& sections,
		       std::size_t section) const;
	// Alto de la celda de un ítem (común o con calidades).
	int cellHeightOf(const MarketItem& item) const;
	// Alto de cada fila de celdas de la sección (el de su celda más alta).
	std::vector<int> rowHeightsOf(const MarketSection& section) const;
	int sectionHeight(const MarketSection& section) const;
	// Caja de texto de una línea de calidad, centrada en vertical.
	PixelBox tierTextBox(const PixelBox& line, int left, int width) const;
};

}
