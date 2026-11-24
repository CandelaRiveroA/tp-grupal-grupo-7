#pragma once

#include <vector>

#include "world/CommunityBoard.h"
#include "world/CommunityCatalog.h"
#include "world/Grid.h"
#include "world/InventoryLayout.h"
#include "world/PixelBox.h"

namespace game {

// Estado de presentación del popup del Centro Comunitario: si está
// abierto y de qué tamaño es su panel. Quien lo pide (hoy la tecla, mañana
// lo que se decida) solo llama a toggle/close. Qué se muestra lo resuelve
// CommunityBoardResolver cada frame, porque el servidor puede cambiar los
// números con el popup abierto.
class CommunityPopupState {
public:
	CommunityPopupState(const world::CommunityCatalog& catalog,
			    const world::InventoryLayout& inventory);

	void toggle();
	void close();
	bool isOpen() const;

	// Grilla del panel (columnas x filas de slots) y su caja centrada en
	// la ventana. Las filas salen del contenido a mostrar.
	world::GridSpan panelGrid(
		const std::vector<world::CommunitySection>& sections) const;
	world::PixelBox panel(
		const std::vector<world::CommunitySection>& sections,
		int windowWidth, int windowHeight) const;

private:
	const world::CommunityCatalog& catalog_;
	const world::InventoryLayout& inventory_;
	bool open_ = false;
};

}
