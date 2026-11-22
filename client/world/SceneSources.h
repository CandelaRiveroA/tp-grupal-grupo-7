#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/EditorWorldSource.h"
#include "world/SceneSource.h"
#include "world/ScenesRoomSource.h"

namespace world {

// Las fuentes de escenas del cliente y su prioridad. Si hay un mundo
// del editor configurado, sus mapas ganan sobre las salas de scenes.toml
// con el mismo id. Sin ese archivo, el cliente funciona solo con
// scenes.toml, como antes.
class SceneSources {
public:
	// `worldPath` vacío o inexistente = sin mundo del editor. Si existe
	// y es inválido lanza ConfigError.
	SceneSources(const std::string& scenesPath, const std::string& worldPath,
		     int worldScale);

	// Las fuentes por orden de prioridad (prestadas: viven lo que vive
	// este objeto).
	std::vector<const SceneSource*> byPriority() const;

private:
	ScenesRoomSource rooms_;
	std::optional<EditorWorldSource> editor_;
};

}
