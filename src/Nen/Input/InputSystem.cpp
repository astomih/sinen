#include <SDL.h>
#include <cstring>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <imgui_impl_sdl.h>
#include <Nen.hpp>
#include "../Render/RendererHandle.hpp"
namespace nen
{
	bool isHide = false;
	SDL_Cursor *g_cursor = nullptr;

	bool KeyboardState::GetKeyValue(KeyCode keyCode) const
	{
		return mCurrState[static_cast<int>(keyCode)] == 1;
	}

	ButtonState KeyboardState::GetKeyState(KeyCode keyCode) const
	{
		if (mPrevState[static_cast<int>(keyCode)] == 0)
		{
			if (mCurrState[static_cast<int>(keyCode)] == 0)
			{
				return ButtonState::None;
			}
			else
			{
				return ButtonState::Pressed;
			}
		}
		else // Prev state must be 1
		{
			if (mCurrState[static_cast<int>(keyCode)] == 0)
			{
				return ButtonState::Released;
			}
			else
			{
				return ButtonState::Held;
			}
		}
	}

	void MouseState::SetPosition(const Vector2 &pos) const
	{
		SDL_WarpMouseInWindow(RendererHandle::GetRenderer()->GetWindow()->GetSDLWindow(), pos.x, pos.y);
	}
	void MouseState::HideCursor(bool hide) const
	{
		isHide = hide;
		if (hide)
		{
			int result = SDL_ShowCursor(SDL_DISABLE);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}
		else
		{
			SDL_ShowCursor(SDL_ENABLE);
		}
	}
	bool MouseState::GetButtonValue(MouseCode button) const
	{
		return (SDL_BUTTON(static_cast<int>(button)) & mCurrButtons) == 1;
	}

	ButtonState MouseState::GetButtonState(MouseCode button) const
	{
		int mask = SDL_BUTTON(static_cast<int>(button));
		if ((mask & mPrevButtons) == 0)
		{
			if ((mask & mCurrButtons) == 0)
			{
				return ButtonState::None;
			}
			else
			{
				return ButtonState::Pressed;
			}
		}
		else
		{
			if ((mask & mCurrButtons) == 0)
			{
				return ButtonState::Released;
			}
			else
			{
				return ButtonState::Held;
			}
		}
	}

	bool ControllerState::GetButtonValue(GameControllerButton button) const
	{
		return mCurrButtons[static_cast<int>(button)] == 1;
	}

	ButtonState ControllerState::GetButtonState(GameControllerButton button) const
	{
		if (mPrevButtons[static_cast<int>(button)] == 0)
		{
			if (mCurrButtons[static_cast<int>(button)] == 0)
			{
				return ButtonState::None;
			}
			else
			{
				return ButtonState::Pressed;
			}
		}
		else // Prev state must be 1
		{
			if (mCurrButtons[static_cast<int>(button)] == 0)
			{
				return ButtonState::Released;
			}
			else
			{
				return ButtonState::Held;
			}
		}
	}

	bool InputSystem::Initialize()
	{

		mState.Keyboard.mCurrState = SDL_GetKeyboardState(NULL);
		memcpy(mState.Keyboard.mPrevState.data(),
			   mState.Keyboard.mCurrState,
			   SDL_NUM_SCANCODES);

		int x = 0, y = 0;
		if (mState.Mouse.mIsRelative)
		{
			mState.Mouse.mCurrButtons =
				SDL_GetRelativeMouseState(&x, &y);
		}
		else
		{
			mState.Mouse.mCurrButtons =
				SDL_GetMouseState(&x, &y);
		}

		mState.Mouse.mMousePos.x = static_cast<float>(x);
		mState.Mouse.mMousePos.y = static_cast<float>(y);

		// Initialize controller state
		mState.Controller.mIsConnected = mController.Initialize();
		memset(mState.Controller.mCurrButtons, 0,
			   SDL_CONTROLLER_BUTTON_MAX);
		memset(mState.Controller.mPrevButtons, 0,
			   SDL_CONTROLLER_BUTTON_MAX);

		return true;
	}

	void InputSystem::Shutdown()
	{
		SDL_FreeCursor(g_cursor);
		g_cursor = nullptr;
		SDL_SetCursor(NULL);
	}

	void InputSystem::PrepareForUpdate()
	{
		// Copy current state to previous
		// Keyboard
		memcpy(mState.Keyboard.mPrevState.data(),
			   mState.Keyboard.mCurrState,
			   SDL_NUM_SCANCODES);

		// Mouse
		mState.Mouse.mPrevButtons = mState.Mouse.mCurrButtons;
		mState.Mouse.mIsRelative = false;
		mState.Mouse.mScrollWheel = Vector2::Zero;

		// Controller
		memcpy(mState.Controller.mPrevButtons,
			   mState.Controller.mCurrButtons,
			   SDL_CONTROLLER_BUTTON_MAX);
	}

	void InputSystem::Update()
	{
		// Mouse
		int x = 0, y = 0;
		if (mState.Mouse.mIsRelative)
		{
			mState.Mouse.mCurrButtons =
				SDL_GetRelativeMouseState(&x, &y);
		}
		else
		{
			mState.Mouse.mCurrButtons =
				SDL_GetMouseState(&x, &y);
		}

		mState.Mouse.mMousePos.x = static_cast<float>(x);
		mState.Mouse.mMousePos.y = static_cast<float>(y);

		// Controller
		// Buttons
		for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
		{
			mState.Controller.mCurrButtons[i] = mController.GetButton(static_cast<GameControllerButton>(i));
		}

		// Triggers
		mState.Controller.mLeftTrigger =
			Filter1D(mController.GetAxis(GameController::Axis::TRIGGERLEFT));
		mState.Controller.mRightTrigger =
			Filter1D(mController.GetAxis(GameController::Axis::TRIGGERRIGHT));

		// Sticks
		x = mController.GetAxis(GameController::Axis::LEFTX);
		y = -mController.GetAxis(GameController::Axis::LEFTY);
		mState.Controller.mLeftStick = Filter2D(x, y);

		x = mController.GetAxis(GameController::Axis::RIGHTX);
		y = -mController.GetAxis(GameController::Axis::RIGHTY);
		mState.Controller.mRightStick = Filter2D(x, y);

		mState.Mouse.HideCursor(isHide);
	}

	void InputSystem::ProcessEvent(SDL_Event &event)
	{
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
			mState.Mouse.mScrollWheel = Vector2(
				static_cast<float>(event.wheel.x),
				static_cast<float>(event.wheel.y));
			break;
		default:
			break;
		}
	}

	void InputSystem::SetRelativeMouseMode(bool value)
	{
		SDL_bool set = value ? SDL_TRUE : SDL_FALSE;
		SDL_SetRelativeMouseMode(set);

		mState.Mouse.mIsRelative = value;
	}

	float InputSystem::Filter1D(int input)
	{
		// A value < dead zone is interpreted as 0%
		const int deadZone = 250;
		// A value > max value is interpreted as 100%
		const int maxValue = 30000;

		float retVal = 0.0f;

		// Take absolute value of input
		int absValue = input > 0 ? input : -input;
		// Ignore input within dead zone
		if (absValue > deadZone)
		{
			// Compute fractional value between dead zone and max value
			retVal = static_cast<float>(absValue - deadZone) /
					 (maxValue - deadZone);
			// Make sure sign matches original value
			retVal = input > 0 ? retVal : -1.0f * retVal;
			// Clamp between -1.0f and 1.0f
			retVal = Math::Clamp(retVal, -1.0f, 1.0f);
		}

		return retVal;
	}

	Vector2 InputSystem::Filter2D(int inputX, int inputY)
	{
		const float deadZone = 8000.0f;
		const float maxValue = 30000.0f;

		// Make into 2D vector
		Vector2 dir;
		dir.x = static_cast<float>(inputX);
		dir.y = static_cast<float>(inputY);

		float length = dir.Length();

		// If length < deadZone, should be no input
		if (length < deadZone)
		{
			dir = Vector2::Zero;
		}
		else
		{
			// Calculate fractional value between
			// dead zone and max value circles
			float f = (length - deadZone) / (maxValue - deadZone);
			// Clamp f between 0.0f and 1.0f
			f = Math::Clamp(f, 0.0f, 1.0f);
			// Normalize the vector, and then scale it to the
			// fractional value
			dir *= f / length;
		}

		return dir;
	}
}