#pragma once

#include <iosfwd>
#include <string>

#include "world_model/FieldWriter.h"
#include "world_model/World.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class WorldTomlWriter {
public:
	std::string write(const World& world) const;

private:
	void writeMap(std::ostream& out, const std::string& id,
		      const Map& map) const;
	void writeObjects(std::ostream& out, const std::string& mapKey,
			  const Map& map) const;
	void writeMarkers(std::ostream& out, const std::string& mapKey,
			  const Map& map) const;
	void writePortals(std::ostream& out, const std::string& mapKey,
			  const Map& map) const;

	FieldWriter fields_;
	WorldVocabulary vocabulary_;
};

}
