#pragma once
#include "../Input/InputSystem.hpp"
#include "../Color/Color.hpp"
#include "Button.hpp"
#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace nen
{
	class UIScreen
	{
	public:
		UIScreen();
		virtual ~UIScreen();
		// UIScreen subclasses can override these
		virtual void Update(float deltaTime);
		virtual void HandleInput(const InputState &state);
		// Tracks if the UI is active or closing
		enum class State
		{
			Active,
			Closing
		};
		// Set state to closing
		void Close();
		// Get state of UI screen
		State GetState() const { return mState; }
		// Change the title text
		void SetTitle(std::string_view text,
					  const Color &color = Palette::White,
					  int pointSize = 40);
		// Add a button to this screen
		void AddButton(std::string_view name, std::function<void()> onClick);

	protected:
		// Helper to draw a texture
		void DrawTexture(class Shader *shader, class Texture *texture,
						 const Vector2 &offset = Vector2::Zero,
						 float scale = 1.0f);
		// Sets the mouse mode to relative or not
		void SetRelativeMouseMode(bool relative);

		std::shared_ptr<class Font> mFont;

		std::shared_ptr<class Texture> mTitle;
		std::shared_ptr<class Texture> mButtonOn;
		std::shared_ptr<class Texture> mButtonOff;
		std::shared_ptr<class Texture> mBackground;
		// Configure positions
		Vector2 mTitlePos;
		Vector2 mNextButtonPos;
		Vector2 mBGPos;

		// State
		State mState;
		// List of buttons
		std::vector<std::unique_ptr<Button>> mButtons;
	};
}