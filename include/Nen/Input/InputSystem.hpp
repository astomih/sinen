#pragma once
#include "../Math/Vector2.hpp"
#include "KeyCode.hpp"
#include "MouseCode.hpp"
#include "GameControllerButton.hpp"
#include <cstdint>
#include <array>


namespace
{
	struct SDL_GameController;
}
namespace nen
{
	// The different button states
	enum class ButtonState
	{
		None,
		Pressed,
		Released,
		Held
	};

	// Helper for keyboard input
	class KeyboardState
	{
	public:
		// Friend so InputSystem can easily update it
		friend class InputSystem;
		// Get just the boolean true/false value of key
		bool GetKeyValue(KeyCode keyCode) const;
		// Get a state based on current and previous frame
		ButtonState GetKeyState(KeyCode keyCode) const;

	private:
		const uint8_t *mCurrState;
		std::array<uint8_t, static_cast<int>(KeyCode::NUM_KEYCODES)> mPrevState;
	};

	// Helper for mouse input
	class MouseState
	{
	public:
		friend class InputSystem;

		// For mouse position
		const Vector2 &GetPosition() const { return mMousePos; }
		const Vector2 &GetScrollWheel() const { return mScrollWheel; }
		bool IsRelative() const { return mIsRelative; }

		// For buttons
		bool GetButtonValue(MouseCode button) const;
		ButtonState GetButtonState(MouseCode button) const;

	private:
		// Store current mouse position
		Vector2 mMousePos;
		// Motion of scroll wheel
		Vector2 mScrollWheel;
		// Store button data
		uint32_t mCurrButtons;
		uint32_t mPrevButtons;
		// Are we in relative mouse mode
		bool mIsRelative;
	};

	// Helper for controller input
	class ControllerState
	{
	public:
		friend class InputSystem;

		// For buttons
		bool GetButtonValue(GameControllerButton button) const;
		ButtonState GetButtonState(GameControllerButton button) const;

		const Vector2 &GetLeftStick() const { return mLeftStick; }
		const Vector2 &GetRightStick() const { return mRightStick; }
		float GetLeftTrigger() const { return mLeftTrigger; }
		float GetRightTrigger() const { return mRightTrigger; }

		bool GetIsConnected() const { return mIsConnected; }

	private:
		// Current/previous buttons
		uint8_t mCurrButtons[static_cast<int>(GameControllerButton::NUM_GAMECONTROLLER_BUTTON)];
		uint8_t mPrevButtons[static_cast<int>(GameControllerButton::NUM_GAMECONTROLLER_BUTTON)];
		// Left/right sticks
		Vector2 mLeftStick;
		Vector2 mRightStick;
		// Left/right trigger
		float mLeftTrigger;
		float mRightTrigger;
		// Is this controller connected?
		bool mIsConnected;
	};

	// Wrapper that contains current state of input
	struct InputState
	{
		KeyboardState Keyboard;
		MouseState Mouse;
		ControllerState Controller;
	};

	class InputSystem
	{
	public:
		bool Initialize();
		void Shutdown();

		// Called right before SDL_PollEvents loop
		void PrepareForUpdate();
		// Called after SDL_PollEvents loop
		void Update();
		// Called to process an SDL event in input system
		void ProcessEvent(union SDL_Event &event);

		const InputState &GetState() const { return mState; }

		void SetRelativeMouseMode(bool value);

	private:
		float Filter1D(int input);
		Vector2 Filter2D(int inputX, int inputY);
		InputState mState;
		::SDL_GameController* mController;
	};
}
