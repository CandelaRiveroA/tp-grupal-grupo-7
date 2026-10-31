#include <optional>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include "gtest/gtest.h"

#include "sdl/input/InputManager.h"

namespace {

using Command = sdl::InputManager::Command;
using CommandKind = sdl::InputManager::CommandKind;
using sdl::InputManager;

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
    EXPECT_FALSE(input.takeCommand().has_value());
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
    const std::optional<Command> command = input.takeCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(command->kind, CommandKind::UseTool);
    EXPECT_FALSE(input.takeCommand().has_value());

    // Soltar la tecla no dispara de nuevo.
    input.handleEvent(keyUp(SDLK_SPACE));
    EXPECT_FALSE(input.takeCommand().has_value());
}

TEST(InputManagerTest, CollectIsOneShotPerPress) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_RETURN));
    const std::optional<Command> command = input.takeCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(command->kind, CommandKind::Collect);
    EXPECT_FALSE(input.takeCommand().has_value());

    // Soltar la tecla no dispara de nuevo.
    input.handleEvent(keyUp(SDLK_RETURN));
    EXPECT_FALSE(input.takeCommand().has_value());
}

TEST(InputManagerTest, SlotKeysAreOneShotInRange) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_2));
    const std::optional<Command> command = input.takeCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(command->kind, CommandKind::SelectSlot);
    EXPECT_EQ(command->slot, 1);
    EXPECT_FALSE(input.takeCommand().has_value());

    // Fuera del rango 1..slotCount no selecciona nada.
    input.handleEvent(keyDown(SDLK_5));
    EXPECT_FALSE(input.takeCommand().has_value());
    input.handleEvent(keyDown(SDLK_0));
    EXPECT_FALSE(input.takeCommand().has_value());
}

TEST(InputManagerTest, ZeroSlotCountDisablesSlotKeys) {
    InputManager input(0);

    input.handleEvent(keyDown(SDLK_1));
    EXPECT_FALSE(input.takeCommand().has_value());
}

TEST(InputManagerTest, SimultaneousOneShotsAllPending) {
    InputManager input(4);

    input.handleEvent(keyDown(SDLK_SPACE));
    input.handleEvent(keyDown(SDLK_RETURN));
    input.handleEvent(keyDown(SDLK_1));

    // Salen en orden de pulsación.
    const std::optional<Command> first = input.takeCommand();
    const std::optional<Command> second = input.takeCommand();
    const std::optional<Command> third = input.takeCommand();
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    ASSERT_TRUE(third.has_value());
    EXPECT_EQ(first->kind, CommandKind::UseTool);
    EXPECT_EQ(second->kind, CommandKind::Collect);
    EXPECT_EQ(third->kind, CommandKind::SelectSlot);
    EXPECT_EQ(third->slot, 0);
    EXPECT_FALSE(input.takeCommand().has_value());
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

    EXPECT_FALSE(input.takeCommand().has_value());
    EXPECT_FALSE(input.up());
    EXPECT_FALSE(input.down());
    EXPECT_FALSE(input.left());
    EXPECT_FALSE(input.right());
}

}  // namespace
