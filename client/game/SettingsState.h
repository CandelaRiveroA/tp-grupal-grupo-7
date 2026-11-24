#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/SoundCategory.h"

namespace game {

// Estado del popup de configuración (ESC), sin SDL: si está abierto, el
// volumen de cada categoría de audio, qué deslizador se está arrastrando y
// el pedido del código QR. Es TDA: el estado es privado y se cambia por
// métodos. Los volúmenes sobreviven al cierre del popup (valen toda la
// sesión). Cómo se ve lo dice SettingsLayout y cómo suena, sdl::AudioMixer.
class SettingsState {
public:
    // Una entrada por categoría con volumen propio (el valor inicial en %).
    explicit SettingsState(const std::map<world::SoundCategory, int>& initialVolumes);

    bool isOpen() const;
    void toggle();
    void close();

    // Abierto = el mundo queda bloqueado (movimiento y clicks).
    bool modal() const;

    // Fija el volumen de la categoría (limitado a 0-100). Una categoría
    // sin entrada no cambia: no se puede configurar lo que no se declaró.
    void setVolume(world::SoundCategory category, int percent);
    int volume(world::SoundCategory category) const;
    bool isMuted(world::SoundCategory category) const;
    const std::map<world::SoundCategory, int>& volumes() const;

    // Deslizador en arrastre: de que se aprieta la perilla a que se suelta.
    void beginDrag(world::SoundCategory category);
    void endDrag();
    const std::optional<world::SoundCategory>& dragging() const;

    // Pedido del código QR de la partida (botón OK). Hoy nadie lo atiende:
    // el servidor lo consumirá y responderá con setQrCode.
    void requestQr();
    // Devuelve si había un pedido pendiente y lo limpia.
    bool takeQrRequest();

    // Pedido de abandonar la partida (botón de cerrar): lo consume
    // PlaySession para terminar la sesión y volver al lobby.
    void requestLeave();
    // Devuelve si había un pedido pendiente y lo limpia.
    bool takeLeaveRequest();

    // El código que mostrará el espacio reservado del popup (vacío = nada).
    void setQrCode(const std::string& code);
    const std::string& qrCode() const;

private:
    bool open_ = false;
    std::map<world::SoundCategory, int> volumes_;
    std::optional<world::SoundCategory> dragging_;
    bool qrRequested_ = false;
    bool leaveRequested_ = false;
    std::string qrCode_;
};

}  // namespace game
