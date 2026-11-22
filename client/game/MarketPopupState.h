#pragma once

#include <vector>

#include "world/InventoryLayout.h"
#include "world/MarketCatalog.h"
#include "world/MarketOffer.h"
#include "world/MarketOfferResolver.h"
#include "world/MarketPopupStyle.h"
#include "world/PixelBox.h"

namespace game {

// Estado de presentación del popup del catálogo de un comerciante: si
// está abierto, qué muestra y de qué tamaño es su panel. Es el ÚNICO
// camino de apertura: la tecla provisoria (B) y, mañana, la oferta que
// mande el servidor llaman al mismo open(). Qué secciones salen de una
// oferta lo decide world::MarketOfferResolver; el dibujo solo pinta lo
// que este estado expone.
class MarketPopupState {
public:
	MarketPopupState(const world::MarketCatalog& catalog,
			 const world::InventoryLayout& inventory);

	// Abre con la oferta y ajusta el panel a su contenido. No abre si no
	// hay nada para mostrar (comerciante sin catálogo o productos que SDL
	// no conoce).
	void open(const world::MarketOffer& offer);
	void close();
	bool isOpen() const;

	// Lo que se dibuja ahora.
	const std::vector<world::MarketSection>& sections() const;

	// Grilla del panel (columnas x filas de slots) y su caja centrada en
	// la ventana. Las filas salen del contenido de la oferta abierta.
	world::GridSpan panelGrid() const;
	world::PixelBox panel(int windowWidth, int windowHeight) const;

private:
	const world::MarketCatalog& catalog_;
	const world::InventoryLayout& inventory_;
	world::MarketOfferResolver resolver_;
	bool open_ = false;
	std::vector<world::MarketSection> sections_;
	world::GridSpan panelGrid_{0, 0};
};

}
