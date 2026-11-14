#include <platform/input/game_controller.hpp>

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

namespace sinen {
namespace {

class VirtualGamepadTest : public testing::Test {
protected:
  void SetUp() override { ASSERT_TRUE(SDL_Init(SDL_INIT_GAMEPAD)); }
  void TearDown() override { SDL_Quit(); }
};

TEST_F(VirtualGamepadTest, DiscoversEnumeratedIdAndReadsAnalogInput) {
  SDL_VirtualJoystickDesc desc;
  SDL_INIT_INTERFACE(&desc);
  desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
  desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
  desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
  desc.name = "Sinen virtual gamepad";
  const SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
  ASSERT_NE(id, 0u) << SDL_GetError();

  Joystick gamepad;
  ASSERT_TRUE(gamepad.initialize()) << SDL_GetError();
  EXPECT_STREQ(gamepad.getName(), desc.name);
  SDL_Joystick *virtualPad = SDL_GetJoystickFromID(id);
  ASSERT_NE(virtualPad, nullptr);
  ASSERT_TRUE(
      SDL_SetJoystickVirtualAxis(virtualPad, SDL_GAMEPAD_AXIS_LEFTX, 16000));
  ASSERT_TRUE(SDL_SetJoystickVirtualAxis(
      virtualPad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 10000));
  ASSERT_TRUE(SDL_SetJoystickVirtualAxis(
      virtualPad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -16000));
  ASSERT_TRUE(
      SDL_SetJoystickVirtualButton(virtualPad, SDL_GAMEPAD_BUTTON_SOUTH, true));
  SDL_UpdateJoysticks();
  EXPECT_EQ(gamepad.getAxis(Joystick::axis::LEFTX), 16000);
  EXPECT_GT(gamepad.getAxis(Joystick::axis::TRIGGERRIGHT), 16000);
  EXPECT_NEAR(gamepad.getAxis(Joystick::axis::TRIGGERLEFT), 8383, 1);
  EXPECT_EQ(gamepad.getButton(GamepadButton::A), 1);

  ASSERT_TRUE(SDL_DetachVirtualJoystick(id));
  EXPECT_FALSE(gamepad.refresh());
  EXPECT_EQ(gamepad.getAxis(Joystick::axis::LEFTX), 0);
  EXPECT_EQ(gamepad.getButton(GamepadButton::A), 0);

  const SDL_JoystickID reconnected = SDL_AttachVirtualJoystick(&desc);
  ASSERT_NE(reconnected, 0u) << SDL_GetError();
  const auto deadline = SDL_GetTicks() + 1500;
  bool recognized = false;
  while (!(recognized = gamepad.refresh()) && SDL_GetTicks() < deadline) {
    SDL_Delay(10);
  }
  EXPECT_TRUE(recognized);
  EXPECT_STREQ(gamepad.getName(), desc.name);
  EXPECT_TRUE(SDL_DetachVirtualJoystick(reconnected));
  gamepad.shutdown();
}

} // namespace
} // namespace sinen
