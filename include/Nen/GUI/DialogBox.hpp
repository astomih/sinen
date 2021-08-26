#pragma once
#include "UIScreen.hpp"
#ifdef DialogBox
#undef DialogBox
#endif

namespace nen
{
	class DialogBox : public UIScreen
	{
	public:
		DialogBox(std::string_view, std::function<void()> onOK);
		~DialogBox();
	};
}