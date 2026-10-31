#pragma once

#include <random>
#include <vector>

#include "game/SoundCue.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ItemKind.h"
#include "world/ViewDirection.h"

namespace game {

// Huevo puesto por un animal en un tick: DATOS puros. El que llena la
// vista (MockGameClient) lo valida y lo guarda como ítem del suelo.
struct ProducedItem {
	int animalId;
	int cellX;
	int cellY;
	world::ItemKind item;
};

// Datos: dirección -> delta en celdas. Usado por el diccionario de
// movimiento de MockAnimalSimulation (tabla sobre switch, convención §4).
struct DirectionDelta {
	world::Direction direction;
	int deltaX;
	int deltaY;
};


// Actividad de un animal en el tick: decidida por la simulación.
enum class AnimalActivity {
	Resting,
	Stepping,
};

// Estado de UN animal: DATOS puros (sin lógica, convención §1); son los
// que el que llena la vista (mock hoy, server mañana) vuelca a la vista.
struct AnimalRecord {
	int id;
	world::CreatureKind kind;
	int cellX;
	int cellY;
	world::Direction facing;
	bool moving;
	AnimalActivity activity;
	int stepIteration;
	int restTicks;
	world::MoveCadence cadence;
	// Datos de postura: pone un huevo cada layPeriod ticks; la
	// primera postura es nextLayIteration = layPeriod (el mock
	// spawnea antes del primer update). El servidor real fijaría
	// estos valores al spawn con la iteración real.
	int layPeriod;
	int nextLayIteration;
	// Datos del sonido: un cue cada soundPeriod ticks, con gama
	// LARGA al spawn y reasignada tras cada emisión (los animales
	// no se sincronizan entre sí).
	int soundPeriod;
	int nextSoundIteration;
};

// MOCK: simulación de movimiento automático (wander) de animales que hoy
// corre en el "servidor falso" (la maneja MockGameClient). Es lógica pura:
// no conoce SDL ni la vista, solo el mundo en celdas. Con una misma seed es
// determinista (testeable). Al conectar la red, esta lógica pasa al server
// real (ahí pierde el prefijo Mock y desaparece del cliente).
class MockAnimalSimulation {
public:
	MockAnimalSimulation(int gridColumns, int gridRows, unsigned int seed);

	// Spawnea un animal: devuelve false (y no spawnea) si la celda está
	// fuera de la grilla, ocupada por otro animal o por el jugador. El
	// ritmo del paso (cadence) es dato del kind, resuelto por quien
	// spawnea desde el catálogo.
	bool spawn(world::CreatureKind kind, const world::MoveCadence& cadence,
		   world::GridCell cell, int playerCellX, int playerCellY);

	// Avanza la simulación hasta `iteration` (una llamada por tick):
	// - Stepping: al cerrar la ventana del paso (cadence.stepTicks)
	//   pasa a Resting con una pausa aleatoria.
	// - Resting: al terminar la pausa elige dirección; si el destino es
	//   pisable arranca el paso (fija la celda destino y facing); si no,
	//   reintenta en el próximo tick. Ocupado el destino, el animal no
	//   avanza: regla de juego del servidor, no un error.
	void update(int iteration, int playerCellX, int playerCellY);

	const std::vector<AnimalRecord>& animals() const;

	// Huevos puestos desde la ÚLTIMA llamada (drena: la segunda
	// llamada devuelve vacío). Cada uno trae la celda donde el
	// animal estaba al ponerlo y el ítem que define la tabla de
	// postura para su kind.
	std::vector<ProducedItem> takeProducedItems();

	// Sonidos desde la ÚLTIMA llamada (drena igual que los huevos).
	std::vector<ViewSoundEvent> takeSoundEvents();

private:
	DirectionDelta randomDirection();

	int randomRestTicks();

	int randomLayPeriod();

	int randomSoundPeriod();

	// Qué ítem pone este kind, o nullptr si no pone nada (tabla
	// kLays en el .cpp: diccionario, convención §4).
	const world::ItemKind* producedItem(world::CreatureKind kind) const;

	void tryProduce(AnimalRecord& animal, int iteration);

	void tryEmitSound(AnimalRecord& animal, int iteration);

	bool canEnter(int cellX, int cellY, int playerCellX, int playerCellY,
		      const AnimalRecord& self) const;

	void tryStartStep(AnimalRecord& animal, int iteration,
			  int playerCellX, int playerCellY);

	int gridColumns_;
	int gridRows_;

	std::mt19937 random_;

	// Stream de RNG SEPARADO para los sonidos: no consume el
	// random_ del wander/postura, cuyas secuencias deterministas
	// pinean los tests.
	std::mt19937 soundRandom_;

	SoundCueCatalog cues_;

	int nextId_;

	std::vector<AnimalRecord> animals_;

	std::vector<ProducedItem> producedItems_;

	std::vector<ViewSoundEvent> clucks_;
};

}