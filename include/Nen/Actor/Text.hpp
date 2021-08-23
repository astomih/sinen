#pragma once
#include "Actor.hpp"
#include "../Color/Color.hpp"
#include <string>
#include <memory>
namespace nen
{
    class Text : public Actor
    {
    public:
        Text(class Scene &scene);
        void SetText(const std::string &text, const int size = 10) const;
        int GetWidth() const;
        int GetHeight() const;
        void SetColor(const Color &color);

    private:
        std::shared_ptr<class FontComponent> mFont;
    };
}