#pragma once

namespace game {

// Qué muestra el popup del HUD.
enum class PopupView {
    // La pregunta ("¿Terminamos el día?") con sus opciones.
    Question,
    // El avance de "dormir" ("Esperando al resto de jugadores...").
    Waiting,
    // La pregunta del cambio de día ("¿Avanzamos al siguiente día?")
    // con su botón OK, sobre la pantalla en negro.
    NextDay,
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
    // el día?"). La llama PlaySession al pisar la cama (ServerSleep).
    void showQuestion();

    // Punto de ENTRADA de la fuente de estado: muestra el avance de
    // "dormir" ("Esperando al resto de jugadores... (ready/total)").
    // La llama PlaySession con los conteos de ServerSleep. No hay red
    // ni protocolo acá: solo llegan los números.
    void showWaiting(int ready, int total);

    // Punto de ENTRADA: la pregunta del cambio de día. La llama
    // PlaySession cuando la pantalla quedó en negro.
    void showNextDay();

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
