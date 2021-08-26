#pragma once
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include <functional>
namespace nen
{
    class Button
    {
    public:
        Button(const std::string &name, std::shared_ptr<class Font> font,
               std::function<void()> onClick,
               const Vector2 &pos, const Vector2 &dims);
        ~Button();

        // Set the name of the button
        void SetName(const std::string &name);

        // Getters/setters
        std::shared_ptr<class Texture> GetNameTex() { return mNameTex; }
        const Vector2 &GetPosition() const { return mPosition; }
        void SetHighlighted(bool sel) { mHighlighted = sel; }
        bool GetHighlighted() const { return mHighlighted; }

        // Returns true if the point is within the button's bounds
        bool ContainsPoint(const Vector2 &pt) const;
        // Called when button is clicked
        void OnClick();

    private:
        std::function<void()> mOnClick;
        std::string mName;
        std::shared_ptr<Texture> mNameTex;
        std::shared_ptr<class Font> mFont;
        Vector2 mPosition;
        Vector2 mDimensions;
        bool mHighlighted;
    };
}