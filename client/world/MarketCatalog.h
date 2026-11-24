#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/MarketOffer.h"
#include "world/MarketPopupStyle.h"
#include "world/MarketSideNames.h"
#include "world/CropQualityNames.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Cómo se presenta una calidad ([qualities.*] en market.toml): su
// nombre, su estrella (sin estrella = ninguna) y el multiplicador con el
// que la oferta por defecto (mock) deriva su monto del monto base.
struct MarketQualityStyle {
	std::string label;
	std::optional<SheetCell> star;
	double multiplier = 1.0;
};

// Catálogo del mercado: se carga de market.toml en el constructor
// (mismo patrón que NpcCatalog). Es el diccionario de PRESENTACIÓN de lo
// que SDL sabe mostrar: el estilo del popup, el título de cada lado, el
// nombre de cada producto y la oferta por defecto (mock) de cada
// comerciante. QUÉ se muestra de verdad lo decide la fuente de estado
// (world::MarketOffer); esto solo responde "cómo se llama / cómo se ve".
class MarketCatalog {
public:
	explicit MarketCatalog(const std::string& path);

	const MarketPopupStyle& style() const;

	// Título de cada lado ([sides] en market.toml).
	const std::map<MarketSide, std::string>& sideTitles() const;

	// Nombre mostrado de un producto; nullptr si SDL no lo conoce.
	const std::string* labelOf(const std::string& contentId) const;

	// Presentación de una calidad; nullptr si market.toml no la declara.
	const MarketQualityStyle* qualityOf(CropQuality quality) const;

	// Oferta por defecto (mock) del comerciante `npcId` en el estilo
	// `set`: las entradas sin `sets` más las que lo contienen. Vacía si
	// el comerciante no tiene catálogo.
	std::vector<MarketEntry> defaultEntries(const std::string& npcId,
						int set) const;

	// Cantidad de estilos rotativos del comerciante: 1 + el mayor `sets`
	// declarado (mínimo 1; 1 si no tiene catálogo).
	int defaultSetCount(const std::string& npcId) const;

private:
	// Entrada de la oferta por defecto y los estilos en que aparece
	// (vacío = en todos).
	struct DefaultEntry {
		MarketEntry entry;
		std::vector<int> sets;
	};

	void readStyle(const tomlcfg::TomlDocument& doc);
	void readSides(const tomlcfg::TomlDocument& doc);
	void readQualities(const tomlcfg::TomlDocument& doc);
	void readProducts(const tomlcfg::TomlDocument& doc);
	void readMerchants(const tomlcfg::TomlDocument& doc);
	// Una entrada del TOML; con `qualities = true` se expande a una por
	// calidad declarada (el mismo dato que mandará el servidor).
	std::vector<DefaultEntry> readEntries(const tomlcfg::TomlDocument& doc,
					      const std::string& base) const;
	DefaultEntry readEntry(const tomlcfg::TomlDocument& doc,
			      const std::string& base) const;

	MarketSideNames sideNames_;
	CropQualityNames qualityNames_;
	MarketPopupStyle style_;
	std::map<MarketSide, std::string> sideTitles_;
	std::map<CropQuality, MarketQualityStyle> qualities_;
	std::map<std::string, std::string> labels_;
	std::map<std::string, std::vector<DefaultEntry>> defaults_;
};

}
