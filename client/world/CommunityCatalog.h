#pragma once

#include <string>
#include <vector>

#include "world/CommunityBoard.h"
#include "world/CommunityPopupStyle.h"
#include "world/CreatureKindNames.h"
#include "world/CropQualityNames.h"

namespace world {

class TomlConfigReader;

// Una meta (ítem, cultivo con calidades, criatura o moneda) de un set.
struct CommunityGoalDef {
	std::string id;
	std::string label;
	CommunityGoalVisual visual;
};

struct CommunitySetDef {
	// El del protocolo (1 a 5).
	int id = 0;
	std::string title;
	std::string helpText;
	bool donatable = true;
	std::vector<CommunityGoalDef> goals;
};

// Catálogo del Centro Comunitario: se carga de community.toml en el
// constructor. Es el diccionario de PRESENTACIÓN: qué sets, metas, sprites
// y estilo muestra SDL; el progreso lo manda el servidor.
class CommunityCatalog {
public:
	explicit CommunityCatalog(const std::string& path);

	const CommunityPopupStyle& style() const;
	const std::string& title() const;
	const std::string& status() const;
	const std::string& donateLabel() const;
	const std::string& helpCloseHint() const;
	const std::vector<CommunitySetDef>& sets() const;

private:
	void readText(const TomlConfigReader& root);
	void readStyle(const TomlConfigReader& root);
	void readHelpStyle(const TomlConfigReader& popup);
	void readSets(const TomlConfigReader& root);
	CommunitySetDef readSet(const TomlConfigReader& setReader) const;
	CommunityGoalDef readGoal(const TomlConfigReader& goalReader) const;

	CreatureKindNames creatureNames_;
	CropQualityNames qualityNames_;
	CommunityPopupStyle style_;
	std::string title_;
	std::string status_;
	std::string donateLabel_;
	std::string helpCloseHint_;
	std::vector<CommunitySetDef> sets_;
};

}  // namespace world
