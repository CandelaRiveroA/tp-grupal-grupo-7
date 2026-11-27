#pragma once

#include <map>
#include <string>
#include <vector>

#include "world/ContentCatalog.h"
#include "world/ContentTraits.h"
#include "world/CropQuality.h"
#include "world/SpriteVisual.h"

namespace world {

// Ciclo de vida de UN cultivo como datos puros: todas sus apariencias
// (el saco del inventario, la semilla plantada, los crecimientos y las
// calidades de la cosecha). El cropId del cultivo es el contentId de su
// PRODUCTO cosechado; el saco de semillas tiene su propio contentId
// (sackId).
struct CropDefinition {
	// contentId del saco de semillas en el inventario (vocabulario
	// compartido con el servidor; mañana lo fijará él).
	std::string sackId;
	SpriteVisual sack;
	SpriteVisual seed;
	std::vector<SpriteVisual> growth;
	std::map<CropQuality, SpriteVisual> qualities;

	// Rasgo de los contenidos del cultivo (saco y producto): ¿se
	// pueden soltar? Mañana la regla real la valida el servidor.
	bool droppable = false;

	// Etapa final del ciclo plantado (la cosecha): la única que se
	// dibuja según la calidad del cultivo.
	int finalStage() const;

	// Sprite de una etapa de su ciclo de vida plantado: etapa 0 = la
	// semilla; las siguientes = crecimiento (orden de `growth`); la
	// etapa final (finalStage(), la cosecha) se dibuja con la CALIDAD
	// pedida (hoy la base la manda la fuente; mañana el servidor).
	// nullptr si la etapa no existe.
	const SpriteVisual* spriteForStage(int stage, CropQuality quality) const;
};

// Catálogo cropId -> definición. El cropId es también el contentId del
// saco en el inventario (vocabulario compartido con el servidor).
class CropCatalog {
public:
	void setEntry(const std::string& cropId, CropDefinition definition);

	// nullptr si el cultivo no está en el catálogo.
	const CropDefinition* find(const std::string& cropId) const;

	const std::map<std::string, CropDefinition>& entries() const;

	// Registra cada cultivo como DOS contenidos del inventario: su saco
	// de semillas (sackId, con el sprite del saco) y su producto
	// cosechado (el cropId, con el sprite de la cosecha). Así el
	// inventario dibuja ambos sin conocer los cultivos (contentId
	// genérico) y lo cosechado se ve igual que en el mundo. Sacar el
	// registro no borra los datos del cultivo, solo su presencia como
	// contenido.
	void registerContents(ContentCatalog& content,
			      ContentTraits& traits) const;

	// cropId del cultivo cuyo saco tiene este contentId (nullptr si no
	// es un saco conocido): la siembra resuelve el saco del inventario
	// al cultivo que planta.
	const std::string* cropIdForSack(const std::string& sackId) const;

	// Escala de dibujo de la planta plantada sobre la celda de tierra
	// (presentación; 1.0 = su tamaño de diseño, un poco más = menos
	// aire alrededor). La cosecha (la etapa final) va una dimensión
	// menos con `harvestScale`.
	void setPlantScale(double scale);
	double plantScale() const;
	void setHarvestScale(double scale);
	double harvestScale() const;

private:
	std::map<std::string, CropDefinition> entries_;
	double plantScale_ = 1.0;
	double harvestScale_ = 1.0;
};

}  // namespace world
