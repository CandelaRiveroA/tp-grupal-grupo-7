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
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        pointerHeld_ = false;
        return;
    }
    if (!textMode_ && event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        pointerHeld_ = true;
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
    // Mapa del mundo (tecla M). El cierre con ESC se resuelve vía ToggleSettings en PlaySession.
    bindings.insert({SDLK_m, game::InputCommand{Kind::ToggleWorldMap, 0}});
    // DEMO: siguiente escena (lo reemplaza el mapa que mande el server).
    bindings.insert({SDLK_o, game::InputCommand{Kind::CycleMap, 0}});
    // DEMO: catálogo del mercado (lo reemplaza el server).
    bindings.insert({SDLK_b, game::InputCommand{Kind::OpenCatalog, 0}});
    // DEMO: aviso "Inventario lleno" (lo reemplaza el server).
    // DEMO: rota el estilo del catálogo (lo reemplaza el server).
    bindings.insert({SDLK_n, game::InputCommand{Kind::NextMarketSet, 0}});
    // Los teclas de test de enemigos (G/F/H/K/V/J/L) quedaron SIN bind
    // a propósito: los enemigos los manda el server por red y la
    // envoltura MockWorldClient (el disparador de esas pruebas) ya no se
    // superpone; los comandos Test* siguen en InputCommand para cuando
    // vuelva un disparador.
    // Abrir el chatbox: one-shot que consume PlaySession (T).
    bindings.insert({SDLK_t, game::InputCommand{Kind::OpenChat, 0}});
    bindings.insert({SDLK_LSHIFT, game::InputCommand{Kind::ToggleWalking, 0}});
    bindings.insert({SDLK_RSHIFT, game::InputCommand{Kind::ToggleWalking, 0}});
    // ESC fuera del chat: el popup de configuración (con el chat abierto
    // ESC lo resuelve handleTextEvent como ChatCancel).
    bindings.insert({SDLK_ESCAPE, game::InputCommand{Kind::ToggleSettings, 0}});
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

bool InputManager::pointerHeld() const { return pointerHeld_; }

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
