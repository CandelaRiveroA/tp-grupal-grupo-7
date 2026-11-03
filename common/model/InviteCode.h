#pragma once

#include <random>
#include <string>

// Código de invitación de partidas: es dueño de la sintaxis, del vocabulario
// y de la generación. Validar y generar viven juntos para que no puedan
// desincronizarse: el vocabulario se define una sola vez, acá. La fuente de
// azar es el estándar <random>, sin librerías externas, y el motor vive en
// la instancia (nada de estado global ni static).
class InviteCode {
public:
	// La longitud es dato del formato; el límite del input Qt la consulta.
	static constexpr std::size_t kLength = 6;

	InviteCode();

	// true si el texto tiene el formato esperado. Que el código exista en
	// una partida lo dice el servidor; acá solo se valida la sintaxis.
	bool isValid(const std::string& code) const;

	// Código nuevo, con el mismo formato que isValid valida.
	std::string generate();

private:
	std::string codeCharacters;
	std::mt19937 engine;
	std::uniform_int_distribution<std::size_t> characterIndex;
};
