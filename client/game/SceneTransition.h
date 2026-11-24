#pragma once

#include <string>

namespace game {

// La transición de escena "apagado y vuelta a prender": fade a negro,
// en el negro cambian los datos y fade de vuelta. Presentación pura
// (sin SDL): el dibujo es un rect con el alfa que esto expone. Sirve
// para el próximo día, para salir de la habitación o para cualquier
// cambio de escenario, siempre en la misma ventana.
//
// Punto de ENTRADA de la fuente de estado: `changeTo(...)` — "cambio de
// escenario". Lo llama PlaySession cuando llega ServerDay. No conoce
// input ni red.
class SceneTransition {
public:
    // fadeTicks: los ticks de cada mitad del fade (los del arranque).
    explicit SceneTransition(int fadeTicks);

    // Pide el cambio a la escena destino. Durante la salida o la espera
    // en negro, un nuevo mapa actualiza el destino. Se ignora si ya hay una
    // transición en curso (una a la vez), SALVO un pedido con holdAtBlack:
    // ese es el fin del día, que no puede perderse por un fundido
    // simultáneo (p. ej. el del cambio de mapa al desmayarse), así que
    // convierte el fundido en curso en uno que se queda en negro. Con
    // holdAtBlack, al llegar al negro se queda ahí (sin cambiar los datos)
    // hasta release().
    void changeTo(const std::string& scene, bool holdAtBlack = false);

    // Avanza el reloj de la transición (una vez por tick).
    void tick(int iteration);

    bool active() const;

    // ¿Está sostenida en negro esperando release()?
    bool holding() const;

    // Suelta el negro sostenido: cambian los datos y vuelve el fade.
    void release();

    // El negro a dibujar sobre todo lo demás: 0 = nada, 255 = negro
    // total (el instante en que cambian los datos).
    int alpha() const;

    // ¿Llegó el instante del negro (el momento de cambiar los datos)?
    bool swapDue() const;

    // Da por hecho el cambio de datos (una sola vez por transición).
    void consumeSwap();

    // La escena destino del cambio en curso.
    const std::string& scene() const;

private:
    enum class Phase {
        Idle,
        FadingOut,
        Holding,
        FadingIn,
    };

    // Convierte la transición en curso en una sostenida en negro.
    void upgradeToHold(const std::string& scene);

    int fadeTicks_;
    bool holdAtBlack_ = false;
    Phase phase_ = Phase::Idle;
    std::string scene_;
    int startIteration_ = -1;
    // Ticks ya "transcurridos" del fundido de salida cuando se retoma desde
    // un fundido de entrada (el negro sigue desde donde estaba, sin salto).
    int resumeElapsed_ = 0;
    int alpha_ = 0;
    bool swapPending_ = false;
};

}  // namespace game
