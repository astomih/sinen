#include <Nen.hpp>
namespace nen
{
    Button::Button(const std::string &name, std::shared_ptr<Font> font,
                   std::function<void()> onClick,
                   const Vector2 &pos, const Vector2 &dims)
        : mOnClick(onClick), mNameTex(nullptr), mFont(font), mPosition(pos), mDimensions(dims), mHighlighted(false)
    {
        SetName(name);
    }

    Button::~Button()
    {
    }

    void Button::SetName(const std::string &name)
    {
        mName = name;
        mNameTex = mFont->RenderText(mName);
    }

    bool Button::ContainsPoint(const Vector2 &pt) const
    {
        const bool no = pt.x < (mPosition.x - mDimensions.x / 2.0f) ||
                        pt.x > (mPosition.x + mDimensions.x / 2.0f) ||
                        pt.y < (mPosition.y - mDimensions.y / 2.0f) ||
                        pt.y > (mPosition.y + mDimensions.y / 2.0f);
        return no;
    }

    void Button::OnClick()
    {
        if (mOnClick)
        {
            mOnClick();
        }
    }
}