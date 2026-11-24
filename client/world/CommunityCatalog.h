#pragma once

#include <string>
#include <vector>

#include "world/CommunityPopupStyle.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Una meta (ítem) de un set: el id con que la nombra el servidor y el
// nombre mostrado (vacío = la fila muestra solo la cantidad).
struct CommunityGoalDef {
	std::string id;
	std::string label;
};

struct CommunitySetDef {
	// El del protocolo (1 a 5).
	int id = 0;
	std::string title;
	// Va antes de cada número de sus filas ("$" en el dinero).
	std::string amountPrefix;
	std::vector<CommunityGoalDef> goals;
};

// Catálogo del Centro Comunitario: se carga de community.toml en el
// constructor (mismo patrón que MarketCatalog). Es el diccionario de
// PRESENTACIÓN: qué sets y metas SDL sabe mostrar, sus textos y el estilo
// del popup. Los números y los estados los manda la fuente de estado.
class CommunityCatalog {
public:
	explicit CommunityCatalog(const std::string& path);

	const CommunityPopupStyle& style() const;
	const std::string& title() const;
	const std::string& status() const;
	const std::vector<CommunitySetDef>& sets() const;

private:
	void readText(const tomlcfg::TomlDocument& doc);
	void readStyle(const tomlcfg::TomlDocument& doc);
	void readSets(const tomlcfg::TomlDocument& doc);
	CommunitySetDef readSet(const tomlcfg::TomlDocument& doc,
				const std::string& base) const;
	CommunityGoalDef readGoal(const tomlcfg::TomlDocument& doc,
				  const std::string& base) const;

	CommunityPopupStyle style_;
	std::string title_;
	std::string status_;
	std::vector<CommunitySetDef> sets_;
};

}
