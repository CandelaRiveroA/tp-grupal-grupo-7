#pragma once

#include <vector>

#include "world/Color.h"
#include "world/CommunityBoard.h"
#include "world/PixelBox.h"
#include "world/SpriteVisual.h"

namespace world {

// Estilo y geometría del popup del Centro Comunitario (datos puros, sin
// SDL). Las cajas se calculan dentro del panel; el painter solo pinta.
struct CommunityPopupStyle {
	// Columnas de slots del panel; las filas salen del contenido.
	int panelColumns = 0;
	int inset = 0;
	int titleHeight = 0;
	int headerHeight = 0;
	int rowHeight = 0;
	int setGap = 0;
	// Separación entre el checkbox y el nombre, y entre el título del set
	// y su estado.
	int checkboxGap = 0;
	int statusGap = 0;
	// Ancho reservado a la cantidad (alineada a la derecha de la fila).
	int amountWidth = 0;
	int titleSize = 0;
	int headerSize = 0;
	int textSize = 0;
	Color titleColor{0, 0, 0, 255};
	Color headerColor{0, 0, 0, 255};
	Color textColor{0, 0, 0, 255};
	Color amountColor{0, 0, 0, 255};
	SpriteVisual checkboxEmpty;
	SpriteVisual checkboxDone;
	SpriteVisual setDone;

	// Alto que ocupa el contenido (márgenes incluidos): lo que el panel
	// debe poder contener.
	int contentHeight(const std::vector<CommunitySection>& sections) const;

	PixelBox titleBox(const PixelBox& panel,
			  const std::vector<CommunitySection>& sections) const;
	PixelBox headerBox(const PixelBox& panel,
			   const std::vector<CommunitySection>& sections,
			   std::size_t section) const;
	PixelBox rowBox(const PixelBox& panel,
			const std::vector<CommunitySection>& sections,
			std::size_t section, std::size_t row) const;

	// Del título del set: el estado y la insignia comparten caja, a la
	// misma altura, a continuación del texto del título.
	PixelBox statusBox(const PixelBox& header, int titleWidth) const;
	PixelBox badgeBox(const PixelBox& header, int titleWidth) const;

	// De una fila.
	PixelBox checkboxBox(const PixelBox& row) const;
	PixelBox labelBox(const PixelBox& row) const;
	PixelBox amountBox(const PixelBox& row, int textWidth) const;

private:
	int contentTop(const PixelBox& panel,
		       const std::vector<CommunitySection>& sections) const;
	int setTop(const PixelBox& panel,
		   const std::vector<CommunitySection>& sections,
		   std::size_t section) const;
	int setHeight(const CommunitySection& section) const;
};

}
