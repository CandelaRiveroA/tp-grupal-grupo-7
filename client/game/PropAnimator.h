#pragma once

#include <map>
#include <string>

namespace game {

// Estado abierto/cerrado de los objetos animados del mundo (cofres, etc.),
// por id. toggle() es el ÚNICO punto de entrada de estado: hoy lo llama el
// click del cliente; mañana lo podrá llamar el servidor. No lee TOML: la
// cantidad de cuadros y el ritmo los pasa quien lo llama (datos del NPC).
class PropAnimator {
public:
	// Alterna abierto/cerrado. Si el objeto todavía se está animando, se
	// ignora.
	void toggle(const std::string& id, int frameCount, int frameTicks,
		    int iteration);

	// Cuadro actual de cada objeto con estado. Un objeto sin estado se
	// dibuja en el cuadro 0 (cerrado): no aparece en el mapa.
	std::map<std::string, int> frames(int iteration) const;

private:
	struct State {
		bool opening = false;
		int start = 0;
		int frameCount = 1;
		int frameTicks = 1;
	};

	int baseFrame(const State& state, int iteration) const;

	std::map<std::string, State> states_;
};

}
