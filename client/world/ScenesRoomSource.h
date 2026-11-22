#pragma once

#include <string>

#include "world/SceneSource.h"

namespace world {

// Las salas de scenes.toml ([rooms.<id>]) como fuente de escenas. Lee
// el archivo en cada consulta: cambiar de escena es raro y así editar
// el TOML no deja datos viejos.
class ScenesRoomSource : public SceneSource {
public:
	explicit ScenesRoomSource(std::string path);

	bool has(const std::string& id) const override;

	SceneDescription load(const std::string& id) const override;

private:
	std::string path_;
};

}
