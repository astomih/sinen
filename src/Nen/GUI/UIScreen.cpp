#include <Nen.hpp>
namespace nen
{
	UIScreen::UIScreen()
		: mTitle(nullptr),
		  mBackground(nullptr),
		  mTitlePos(0.0f, 300.0f),
		  mNextButtonPos(0.0f, 200.0f),
		  mBGPos(0.0f, 250.0f),
		  mState(State::Active)
	{
	}

	UIScreen::~UIScreen()
	{
		mButtons.clear();
	}

	void UIScreen::HandleInput(const InputState &state)
	{
		// Do we have buttons?
		if (!mButtons.empty())
		{
			// Get position of mouse
			int x, y;
			x = state.Mouse.GetPosition().x;
			y = state.Mouse.GetPosition().y;
			// Convert to (0,0) center coordinates
			Vector2 mousePos((x), (y));
			mousePos.x -= 1280 * 0.5f;
			mousePos.y = 720 * 0.5f - mousePos.y;

			// Highlight any buttons
			for (auto &b : mButtons)
			{
				if (b->ContainsPoint(mousePos))
				{
					b->SetHighlighted(true);
				}
				else
				{
					b->SetHighlighted(false);
				}
			}
		}
	}

	void UIScreen::Close()
	{
		mState = State::Closing;
	}

	void UIScreen::SetTitle(std::string_view text, const Color &color, int pointSize)
	{
		// Clear out previous title texture if it exists
		const auto font = mFont->RenderText(text.data(), color);
	}

	void UIScreen::AddButton(std::string_view name, std::function<void()> onClick)
	{
		Vector2 dims((mButtonOn->GetWidth()),
					 (mButtonOn->GetHeight()));
		auto b = std::make_unique<Button>(name.data(), mFont, onClick, mNextButtonPos, dims);
		mButtons.emplace_back(std::move(b));

		mNextButtonPos.y -= mButtonOff->GetHeight() + 20.0f;
	}

	void UIScreen::SetRelativeMouseMode(bool relative)
	{
	}

}