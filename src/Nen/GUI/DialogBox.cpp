#include <Nen.hpp>

namespace nen
{
	DialogBox::DialogBox(std::string_view text, std::function<void()> onOK)
		: UIScreen()
	{
		// Adjust positions for dialog box
		mBGPos = Vector2(0.0f, 0.0f);
		mTitlePos = Vector2(0.0f, 100.0f);
		mNextButtonPos = Vector2(0.0f, 0.0f);
		SetTitle(text.data(), Palette::Black, 30);
		AddButton("OK", [onOK]()
				  { onOK(); });
		AddButton("Cancel", [this]()
				  { Close(); });
	}

	DialogBox::~DialogBox()
	{
	}
}