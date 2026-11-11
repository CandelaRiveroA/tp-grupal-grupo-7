#pragma once

#include <string>

#include "world/PromptLayout.h"

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
    // El que da un regalo: "¿Queres darle este regalo?".
    GiftOffer,
    // El que espera la respuesta del otro jugador.
    GiftWaiting,
    // El que recibe un regalo: "[jugador] te quiere hacer un regalo...".
    GiftReceived,
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

    // Punto de ENTRADA del flujo de regalos. GiftOffer lo dispara el
    // click sobre otro jugador; GiftWaiting y GiftReceived llegarán
    // del server (hoy los simulan las teclas demo).
    void showGiftOffer();
    void showGiftWaiting();
    void showGiftReceived(const std::string& senderName);

    // Sin popup (cualquier opción lo cierra: es una respuesta; la
    // consecuencia la decidirá la fuente de estado).
    void hide();

    // ¿Mientras se muestra, el mundo está bloqueado? El popup espera
    // respuesta: solo se puede responder o cerrar, nada más. Es la
    // decisión del popup (hoy todo popup visible es modal); el input
    // la respeta, sin saber de qué popup se trata.
    bool modal() const;

    PopupView view() const;

    // La copia del popup actual en ui/text.toml. El painter y el
    // hit-testing usan la MISMA, así lo que se ve y lo que se toca no
    // se desincronizan.
    world::PromptId promptId() const;

    // Nombre que manda la fuente para el popup del receptor ({0}).
    const std::string& giftSenderName() const;

    int waitingReady() const;

    int waitingTotal() const;

private:
    PopupView view_ = PopupView::Hidden;
    world::PromptId promptId_ = world::PromptId::SleepQuestion;
    int waitingReady_ = 0;
    int waitingTotal_ = 0;
    std::string giftSenderName_;
};

}  // namespace game
