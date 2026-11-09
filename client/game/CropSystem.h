#pragma once

#include <string>
#include <vector>

#include "game/Interactor.h"
#include "game/LocalWorldView.h"
#include "world/CropCatalog.h"

namespace game {

// Los cultivos plantados del mundo simulado: siembra, crecimiento por
// riego y cosecha levantable. Es uno más de los Interactor del
// orquestador (como AnimalSystem y FloorItems): la cosecha lista se
// levanta por el camino genérico de recoger — la misma utilidad que el
// huevo — y lo que levanta es lo que entra al inventario (el producto,
// dibujado como la cosecha).
//
// DEMO (borrar cuando el servidor mande etapas, pedidos de agua y
// cosechas): las reglas de hoy simulan las suyas.
class CropSystem : public Interactor {
public:
	// crops: catálogo PRESTADO (de ahí salen las reglas del ciclo:
	// la etapa final). nullptr = sin demo.
	explicit CropSystem(const world::CropCatalog* crops);

	// Siembra la semilla (stage 0) pidiendo agua, una vez por celda.
	void plant(int cellX, int cellY, const std::string& cropId);

	// El riego que atiende el pedido de agua de un cultivo lo hace
	// crecer una etapa ("según corresponda": el servidor decidirá el
	// avance real). En la etapa final queda con la calidad base y
	// listo para levantar.
	void waterAt(int cellX, int cellY);

	// Los cultivos plantados (para la vista).
	const std::vector<ViewCrop>& planted() const;

	// --- Interactor: la cosecha se levanta como el huevo -----------
	std::optional<InteractionTarget> findTargetNear(
		int cellX, int cellY) const override;
	void onInteractionStarted(const InteractionTarget& target) override;
	void onInteractionFinished(const InteractionTarget& target) override;

private:
	const world::CropCatalog* crops_;
	std::vector<ViewCrop> planted_;
};

}
