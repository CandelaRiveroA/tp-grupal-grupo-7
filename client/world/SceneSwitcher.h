#pragma once

#include <string>
#include <vector>

#include "world/SceneDescription.h"
#include "world/SceneSource.h"

namespace world {

// Dueño de la escena ACTUAL y de cómo se resuelve un cambio de escena.
// Datos puros (sin SDL): quien lo usa (PlaySession) aplica el resultado
// al renderer, la cámara y las selecciones. Las fuentes se consultan
// por orden de prioridad: gana la primera que conoce el id.
class SceneSwitcher {
public:
	// Las fuentes son PRESTADAS: deben vivir más que el switcher.
	SceneSwitcher(std::vector<const SceneSource*> sources,
		      std::string initialId, SceneDescription initialScene);

	const SceneDescription& scene() const;

	// Id de la escena actual (sala o mapa).
	const std::string& id() const;

	// Carga la escena `id` y la deja como actual. Si ninguna fuente la
	// conoce lanza ConfigError(MissingStructure, ...) y la actual no
	// cambia.
	void switchTo(const std::string& id);

	// La escena que sigue a la actual recorriendo, por orden de
	// prioridad de las fuentes, los ids de todas (sin repetir los que
	// dos fuentes comparten). Vuelve a la primera tras la última. Si la
	// actual no está en la lista, devuelve la primera. Sin escenas
	// conocidas devuelve la actual.
	std::string nextId() const;

private:
	std::vector<std::string> allIds() const;

	std::vector<const SceneSource*> sources_;
	std::string id_;
	SceneDescription scene_;
};

}
