#include "InputManager.h"

#include <algorithm>

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

namespace sdl {

InputManager::InputManager(int slotCount):
        slotKeys_{SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0},
        slotCount_(slotCount) {
    keymap_ = defaultBindings();
    commandBindings_ = defaultCommandBindings();
    // Prioridad al leer varias teclas a la vez: left, right, up, down
    // (el mismo orden que usaba la lectura original del input).
    directionBindings_ = {{
            {Action::Left, world::Direction::Left},
            {Action::Right, world::Direction::Right},
            {Action::Up, world::Direction::Back},
            {Action::Down, world::Direction::Front},
    }};
}

bool InputManager::up() const { return flags_[static_cast<std::size_t>(Action::Up)]; }

bool InputManager::down() const { return flags_[static_cast<std::size_t>(Action::Down)]; }

bool InputManager::left() const { return flags_[static_cast<std::size_t>(Action::Left)]; }

bool InputManager::right() const { return flags_[static_cast<std::size_t>(Action::Right)]; }

auto InputManager::defaultBindings() const -> std::unordered_map<SDL_Keycode, Action> {

    return {
            {SDLK_UP, Action::Up},       {SDLK_w, Action::Up},      {SDLK_DOWN, Action::Down},
            {SDLK_s, Action::Down},      {SDLK_LEFT, Action::Left}, {SDLK_a, Action::Left},
            {SDLK_RIGHT, Action::Right}, {SDLK_d, Action::Right},
    };
}

void InputManager::handleEvent(const SDL_Event& event) {

    // El puntero: su posición se actualiza con el movimiento del mouse
    // (el dibujo del cursor la consulta cada frame).
    if (event.type == SDL_MOUSEMOTION) {
        pointerPosition_ = game::PointerPosition{event.motion.x, event.motion.y};
        return;
    }

    // Clicks: solo con el chat cerrado (sin input de juego no significan
    // nada).
    if (!textMode_ && event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        clicks_.push_back(game::PointerClick{event.button.x, event.button.y});
        return;
    }

    // Modo texto: caracteres, backspace y los one-shots Enter/ESC.
    if (textMode_) {
        handleTextEvent(event);
        return;
    }

    if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
        return;
    }

    if (event.key.repeat != 0) {
        return;
    }

    const SDL_Keycode sym = event.key.keysym.sym;

    // Los comandos one-shot se disparan solo con la pulsación (keydown);
    // la suelta no dispara nada (cae al mapeo de movimiento, que la ignora
    // si la tecla no está bound).
    if (event.type == SDL_KEYDOWN) {
        if (const game::InputCommand* command = commandFor(sym)) {
            fire(*command);
            return;
        }
    }

    setKey(sym, event.type == SDL_KEYDOWN);
}

auto InputManager::defaultCommandBindings() const
        -> std::unordered_map<SDL_Keycode, game::InputCommand> {

    using Kind = game::InputCommand::Kind;
    std::unordered_map<SDL_Keycode, game::InputCommand> bindings;
    // Orden = prioridad: ante colisiones de tecla, gana el primer comando.
    // "Usar herramienta" ya no se dispara con tecla: lo dispara el
    // click sobre el mundo (PlayerController::useToward). Si algún día
    // se rebindea a una tecla, vuelve a entrar acá.
    bindings.insert({SDLK_RETURN, game::InputCommand{Kind::Collect, 0}});
    // Los teclas de test de enemigos (G/F/H/K/V/J/L) quedaron SIN bind
    // a propósito: los enemigos los manda el server por red y la
    // envoltura MockWorldClient (el disparador de esas pruebas) ya no se
    // superpone; los comandos Test* siguen en InputCommand para cuando
    // vuelva un disparador.
    // Demo del gasto de energía: cada pulsación simula una acción que
    // cansa al jugador (gastar). Es un one-shot rebindeable; cuando el
    // server mande el gasto real, esta tecla se puede quitar sin tocar
    // el dibujo de la barra.
    bindings.insert({SDLK_u, game::InputCommand{Kind::SpendEnergy, 0}});
    // Demo del gasto de vida: mismo mecanismo que la energía, con otra
    // tecla (I). Cuando el server mande la vida real, esta tecla se puede
    // quitar sin tocar el dibujo de la barra.
    bindings.insert({SDLK_i, game::InputCommand{Kind::SpendLife, 0}});
    // Demo del desmayo: one-shot local que dispara la secuencia de
    // Faint + el diálogo de las Z. Cuando el server mande la acción,
    // esta tecla se puede quitar sin tocar el dibujo.
    bindings.insert({SDLK_o, game::InputCommand{Kind::Faint, 0}});
    // Demo del daño recibido: one-shot local que dispara la titilación
    // del jugador (aparece/desaparece). Cuando el server mande el daño,
    // esta tecla se puede quitar sin tocar el dibujo.
    bindings.insert({SDLK_r, game::InputCommand{Kind::Hurt, 0}});
    // Demo del charco de sangre: one-shot local que deja un charco en
    // la celda del jugador. Cuando la fuente mande sangre, se quita.
    bindings.insert({SDLK_c, game::InputCommand{Kind::BloodPuddle, 0}});
    // Abrir el chatbox: one-shot que consume PlaySession (T).
    bindings.insert({SDLK_t, game::InputCommand{Kind::OpenChat, 0}});
    // Demo del chat (M): como U/I/O, se quita cuando el server mande el
    // chat.
    bindings.insert({SDLK_m, game::InputCommand{Kind::SimulateChatMessage, 0}});
    const int bound = std::min(slotCount_, static_cast<int>(slotKeys_.size()));
    for (int i = 0; i < bound; ++i) {
        bindings.insert(
                {slotKeys_[static_cast<std::size_t>(i)], game::InputCommand{Kind::SelectSlot, i}});
    }
    return bindings;
}

auto InputManager::commandFor(SDL_Keycode key) const -> const game::InputCommand* {
    if (const auto it = commandBindings_.find(key); it != commandBindings_.end()) {
        return &it->second;
    }
    return nullptr;
}

void InputManager::fire(const game::InputCommand& command) { pending_.push_back(command); }

std::vector<game::InputCommand> InputManager::takeCommands() {
    std::vector<game::InputCommand> commands(pending_.begin(), pending_.end());
    pending_.clear();
    return commands;
}

std::vector<game::PointerClick> InputManager::takeClicks() {
    std::vector<game::PointerClick> clicks;
    clicks.swap(clicks_);
    return clicks;
}

void InputManager::setTextMode(bool open) {
    textMode_ = open;
    if (open) {
        // El chat toma el input: se sueltan las teclas de movimiento
        // held.
        flags_ = {};
        return;
    }
    textPending_.clear();
    backspacePending_ = 0;
}

void InputManager::handleTextEvent(const SDL_Event& event) {
    if (event.type == SDL_TEXTINPUT) {
        textPending_ += event.text.text;
        return;
    }

    if (event.type != SDL_KEYDOWN) {
        return;
    }

    // El backspace con repeat borra en cadena; Enter/ESC solo la primera.
    if (event.key.keysym.sym == SDLK_BACKSPACE) {
        ++backspacePending_;
        return;
    }
    if (event.key.repeat != 0) {
        return;
    }
    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
        pending_.push_back(game::InputCommand{game::InputCommand::Kind::ChatSubmit, 0});
    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
        pending_.push_back(game::InputCommand{game::InputCommand::Kind::ChatCancel, 0});
    }
}

std::string InputManager::takeText() {
    std::string taken = textPending_;
    textPending_.clear();
    return taken;
}

int InputManager::takeBackspaces() {
    const int taken = backspacePending_;
    backspacePending_ = 0;
    return taken;
}

const game::PointerPosition& InputManager::mousePosition() const { return pointerPosition_; }

std::optional<world::Direction> InputManager::pressedDirection() const {
    for (const DirectionBinding& binding: directionBindings_) {
        if (flags_[static_cast<std::size_t>(binding.action)]) {
            return binding.direction;
        }
    }
    return std::nullopt;
}

void InputManager::setKey(SDL_Keycode key, bool pressed) {

    if (auto it = keymap_.find(key); it != keymap_.end()) {
        flags_[static_cast<std::size_t>(it->second)] = pressed;
    }
}

}  // namespace sdl
