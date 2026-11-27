#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/ConfigError.h"
#include "world/EditorWorldSource.h"
#include "world/SceneSource.h"
#include "world/ScenesRoomSource.h"

namespace world {

// Las fuentes de escenas del cliente y su prioridad. En una partida de red
// se usa el mundo del editor; la fuente legacy de scenes.toml queda
// disponible solo para consumidores que la soliciten explícitamente.
class SceneSources {
public:
	// `worldPath` vacío o inexistente = sin mundo del editor. Si existe
	// y es inválido lanza ConfigError. `scenesPath` vacío deshabilita la
	// fuente legacy. `overlayPath` completa el mundo si existe.
	SceneSources(const std::string& scenesPath, const std::string& worldPath,
		     const std::string& overlayPath);

	// Entradas del overlay sin coincidencia (vacío sin mundo del editor).
	std::vector<ConfigError> findings() const;

	// Las fuentes por orden de prioridad (prestadas: viven lo que vive
	// este objeto).
	std::vector<const SceneSource*> byPriority() const;

private:
	ScenesRoomSource rooms_;
	bool roomsEnabled_;
	std::optional<EditorWorldSource> editor_;
};

}
