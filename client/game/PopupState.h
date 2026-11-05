#pragma once

namespace game {

// Qué muestra el popup del HUD.
enum class PopupView {
	// La pregunta ("¿Terminamos el día?") con sus opciones.
	Question,
	// El avance de "dormir" ("Esperando al resto de jugadores...").
	Waiting,
	// Sin popup.
	Hidden,
};

// Estado de presentación local de un popup del HUD. Es presentación,
// como FarmSelection: la fuente de estado no participa del dibujo. Lo
// que pasa cuando se contesta la pregunta lo decidirá el server (fuera
// de alcance); acá solo se ve.
class PopupState {
public:
	// Punto de ENTRADA de la fuente de estado: la pregunta ("¿Terminamos
	// el día?"). Mañana es el server quien la llama cuando corresponde
	// preguntar; hoy la llama el click en la cama, un stand-in temporal
	// (el mismo rol que la tecla Y para showWaiting).
	void showQuestion();

	// Punto de ENTRADA de la fuente de estado: muestra el avance de
	// "dormir" ("Esperando al resto de jugadores... (ready/total)").
	// Hoy lo llama el atajo de prueba (tecla Y); mañana lo llama el
	// GameClient de red cuando el server mande esta información. No
	// hay red ni protocolo acá: solo llegan los números.
	void showWaiting(int ready, int total);

	// Sin popup (cualquier opción de la pregunta lo cierra: es un
	// voto; la consecuencia la decidirá la fuente de estado).
	void hide();

	// ¿Mientras se muestra, el mundo está bloqueado? El popup espera
	// respuesta: solo se puede responder o cerrar, nada más. Es la
	// decisión del popup (hoy todo popup visible es modal); el input
	// la respeta, sin saber de qué popup se trata.
	bool modal() const;

	PopupView view() const;

	int waitingReady() const;

	int waitingTotal() const;

private:
	PopupView view_ = PopupView::Hidden;
	int waitingReady_ = 0;
	int waitingTotal_ = 0;
};

}  // namespace game
