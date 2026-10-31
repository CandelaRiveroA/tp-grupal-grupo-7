#include <optional>
#include <vector>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include "gtest/gtest.h"

#include "game/InputCommand.h"
#include "sdl/input/InputManager.h"
#include "world/ViewDirection.h"

namespace {

using Command = game::InputCommand;
using CommandKind = game::InputCommand::Kind;
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

TEST(InputManagerTest, RepeatKeyEventsAreIgnored) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_UP, 1));
    EXPECT_FALSE(input.up());

    input.handleEvent(keyDown(SDLK_SPACE, 1));
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

TEST(InputManagerTest, UseToolIsOneShotPerPress) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_SPACE));
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].kind, CommandKind::UseTool);
    EXPECT_TRUE(input.takeCommands().empty());

    // Soltar la tecla no dispara de nuevo.
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

TEST(InputManagerTest, SimultaneousOneShotsAllPending) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_SPACE));
    input.handleEvent(keyDown(SDLK_RETURN));
    input.handleEvent(keyDown(SDLK_1));

    // Salen juntos y en orden de pulsación.
    const std::vector<Command> commands = input.takeCommands();
    ASSERT_EQ(commands.size(), 3u);
    EXPECT_EQ(commands[0].kind, CommandKind::UseTool);
    EXPECT_EQ(commands[1].kind, CommandKind::Collect);
    EXPECT_EQ(commands[2].kind, CommandKind::SelectSlot);
    EXPECT_EQ(commands[2].slot, 0);
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
