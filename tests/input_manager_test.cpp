#include <array>
#include <optional>
#include <vector>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

#include "gtest/gtest.h"

#include "game/InputCommand.h"
#include "game/PointerClick.h"
#include "sdl/input/InputManager.h"
#include "world/ViewDirection.h"

namespace {

using Command = game::InputCommand;
using CommandKind = game::InputCommand::Kind;
using game::PointerClick;
using sdl::InputManager;
using world::Direction;

SDL_Event keyEvent(Uint32 type, SDL_Keycode sym, Uint8 repeat = 0) {
    SDL_Event event{};
    event.type = type;
    event.key.keysym.sym = sym;
    event.key.repeat = repeat;
    return event;
}

SDL_Event keyDown(SDL_Keycode sym, Uint8 repeat = 0) {
    return keyEvent(SDL_KEYDOWN, sym, repeat);
}

SDL_Event keyUp(SDL_Keycode sym) {
    return keyEvent(SDL_KEYUP, sym, 0);
}

SDL_Event mouseButtonDown(int x, int y, Uint8 button = SDL_BUTTON_LEFT) {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = button;
    event.button.x = x;
    event.button.y = y;
    return event;
}

SDL_Event mouseButtonUp(int x, int y, Uint8 button = SDL_BUTTON_LEFT) {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONUP;
    event.button.button = button;
    event.button.x = x;
    event.button.y = y;
    return event;
}

TEST(InputManagerTest, RepeatKeyEventsAreIgnored) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_UP, 1));
    EXPECT_FALSE(input.up());

    input.handleEvent(keyDown(SDLK_RETURN, 1));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, MovementFlagsFollowKeyState) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_UP));
    EXPECT_TRUE(input.up());

    input.handleEvent(keyUp(SDLK_UP));
    EXPECT_FALSE(input.up());

    // Alias: W también es Up.
    input.handleEvent(keyDown(SDLK_w));
    EXPECT_TRUE(input.up());

    // Tecla sin bind no mueve nada.
    input.handleEvent(keyDown(SDLK_x));
    EXPECT_FALSE(input.down());
    EXPECT_FALSE(input.left());
    EXPECT_FALSE(input.right());
}

// "Usar herramienta" ya no se dispara con tecla: lo dispara el click
// sobre el mundo (PlayerController::useToward). Espacio no manda nada.
TEST(InputManagerTest, SpaceFiresNoCommands) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_SPACE));
    EXPECT_TRUE(input.takeCommands().empty());

    // Soltar la tecla tampoco.
    input.handleEvent(keyUp(SDLK_SPACE));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, CollectIsOneShotPerPress) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_RETURN));
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].kind, CommandKind::Collect);
    EXPECT_TRUE(input.takeCommands().empty());

    // Soltar la tecla no dispara de nuevo.
    input.handleEvent(keyUp(SDLK_RETURN));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, FaintIsOneShotPerPress) {
    InputManager input(4);

    // O = demo del desmayo (la F quedó en los test de enemigos).
    input.handleEvent(keyDown(SDLK_o));
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].kind, CommandKind::Faint);

    // Soltar no dispara de nuevo.
    input.handleEvent(keyUp(SDLK_o));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, SlotKeysAreOneShotInRange) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_2));
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].kind, CommandKind::SelectSlot);
    EXPECT_EQ(commands[0].slot, 1);
    EXPECT_TRUE(input.takeCommands().empty());

    // Fuera del rango 1..slotCount no selecciona nada.
    input.handleEvent(keyDown(SDLK_5));
    EXPECT_TRUE(input.takeCommands().empty());
    input.handleEvent(keyDown(SDLK_0));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, ZeroSlotCountDisablesSlotKeys) {
    InputManager input(0);

    input.handleEvent(keyDown(SDLK_1));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, SlotKeysFollowTheKeyTable) {
    InputManager input(10);

    // La tabla de teclas: un dígito por slot (1..9 y luego el 0).
    const std::array<SDL_Keycode, 10> slotKeys = {
        SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
        SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0,
    };
    for (int i = 0; i < 10; ++i) {
        input.handleEvent(keyDown(slotKeys[static_cast<std::size_t>(i)]));
        const std::vector<Command> commands = input.takeCommands();
        ASSERT_EQ(commands.size(), 1u);
        EXPECT_EQ(commands[0].kind, CommandKind::SelectSlot);
        EXPECT_EQ(commands[0].slot, i);
    }
}

TEST(InputManagerTest, SlotKeysStopAtTheTableSize) {
    // Más slots que teclas: los slots sin tecla quedan sin binding (la
    // selección de esos slots vendrá por otro input), no se inventan
    // teclas con aritmética fuera de rango.
    InputManager input(12);

    input.handleEvent(keyDown(static_cast<SDL_Keycode>(SDLK_1 + 9)));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, ZeroKeyIsSlotNineOnlyWhenThereAreTenSlots) {
    InputManager input(9);
    input.handleEvent(keyDown(SDLK_0));
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, SimultaneousOneShotsAllPending) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_RETURN));
    input.handleEvent(keyDown(SDLK_1));
    input.handleEvent(keyDown(SDLK_2));

    // Salen juntos y en orden de pulsación.
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 3u);
    EXPECT_EQ(commands[0].kind, CommandKind::Collect);
    EXPECT_EQ(commands[1].kind, CommandKind::SelectSlot);
    EXPECT_EQ(commands[1].slot, 0);
    EXPECT_EQ(commands[2].kind, CommandKind::SelectSlot);
    EXPECT_EQ(commands[2].slot, 1);
    EXPECT_TRUE(input.takeCommands().empty());
}

TEST(InputManagerTest, PressedDirectionFollowsMovementKeys) {
    InputManager input(4);

    EXPECT_FALSE(input.pressedDirection().has_value());

    input.handleEvent(keyDown(SDLK_UP));
    EXPECT_EQ(input.pressedDirection(), Direction::Back);

    // Prioridad: Left gana si hay varias teclas pulsadas.
    input.handleEvent(keyDown(SDLK_LEFT));
    EXPECT_EQ(input.pressedDirection(), Direction::Left);
    input.handleEvent(keyUp(SDLK_LEFT));
    EXPECT_EQ(input.pressedDirection(), Direction::Back);

    input.handleEvent(keyUp(SDLK_UP));
    input.handleEvent(keyDown(SDLK_DOWN));
    EXPECT_EQ(input.pressedDirection(), Direction::Front);

    input.handleEvent(keyUp(SDLK_DOWN));
    input.handleEvent(keyDown(SDLK_RIGHT));
    EXPECT_EQ(input.pressedDirection(), Direction::Right);

    // Alias: W también es Up (Back).
    input.handleEvent(keyUp(SDLK_RIGHT));
    input.handleEvent(keyDown(SDLK_w));
    EXPECT_EQ(input.pressedDirection(), Direction::Back);
}

TEST(InputManagerTest, MouseMotionTracksPointerPosition) {
    InputManager input(4);

    // Sin movimiento, el puntero arranca en el origen.
    EXPECT_EQ(input.mousePosition().x, 0);
    EXPECT_EQ(input.mousePosition().y, 0);

    SDL_Event motion{};
    motion.type = SDL_MOUSEMOTION;
    motion.motion.x = 320;
    motion.motion.y = 240;
    input.handleEvent(motion);
    EXPECT_EQ(input.mousePosition().x, 320);
    EXPECT_EQ(input.mousePosition().y, 240);

    // El teclado no mueve el puntero.
    input.handleEvent(keyDown(SDLK_UP));
    EXPECT_EQ(input.mousePosition().x, 320);
    EXPECT_EQ(input.mousePosition().y, 240);
}

TEST(InputManagerTest, MouseClickCarriesPosition) {
    InputManager input(4);

    input.handleEvent(mouseButtonDown(320, 240));
    const std::vector<PointerClick> clicks = input.takeClicks();
    ASSERT_EQ(clicks.size(), 1u);
    EXPECT_EQ(clicks[0].x, 320);
    EXPECT_EQ(clicks[0].y, 240);
    EXPECT_TRUE(input.takeClicks().empty());
}

TEST(InputManagerTest, ClickIsOneShotPerPress) {
    InputManager input(4);

    input.handleEvent(mouseButtonDown(10, 10));
    input.handleEvent(mouseButtonUp(10, 10));
    // Soltar el botón no genera otro click.
    ASSERT_EQ(input.takeClicks().size(), 1u);

    // Varios clicks en el mismo frame salen juntos y en orden.
    input.handleEvent(mouseButtonDown(1, 1));
    input.handleEvent(mouseButtonDown(2, 2));
    const std::vector<PointerClick> clicks = input.takeClicks();
    ASSERT_EQ(clicks.size(), 2u);
    EXPECT_EQ(clicks[0].x, 1);
    EXPECT_EQ(clicks[1].x, 2);
}

TEST(InputManagerTest, OnlyPrimaryButtonClicks) {
    InputManager input(4);

    input.handleEvent(mouseButtonDown(5, 5, SDL_BUTTON_RIGHT));
    input.handleEvent(mouseButtonDown(5, 5, SDL_BUTTON_MIDDLE));
    EXPECT_TRUE(input.takeClicks().empty());
}

TEST(InputManagerTest, KeyboardAndMotionDoNotGenerateClicks) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_SPACE));
    input.handleEvent(keyUp(SDLK_SPACE));
    SDL_Event motion{};
    motion.type = SDL_MOUSEMOTION;
    motion.motion.x = 10;
    motion.motion.y = 20;
    input.handleEvent(motion);
    EXPECT_TRUE(input.takeClicks().empty());
}

TEST(InputManagerTest, NonKeyboardEventsAreIgnored) {
    InputManager input(4);

    SDL_Event motion{};
    motion.type = SDL_MOUSEMOTION;
    motion.motion.x = 10;
    motion.motion.y = 20;
    input.handleEvent(motion);

    SDL_Event quit{};
    quit.type = SDL_QUIT;
    input.handleEvent(quit);

    EXPECT_TRUE(input.takeCommands().empty());
    EXPECT_FALSE(input.pressedDirection().has_value());
    EXPECT_FALSE(input.up());
    EXPECT_FALSE(input.down());
    EXPECT_FALSE(input.left());
    EXPECT_FALSE(input.right());
}

}  // namespace
