#pragma once
#include <Math/Vector3.hpp>
namespace nen
{
    class Color
    {
    public:
        constexpr Color(const float red, const float green, const float blue, const float alpha)
            : r(red), g(green), b(blue), a(alpha)
        {
        }
        constexpr Color(const float value)
            : r(value), g(value), b(value), a(1.f)
        {
        }

        float r;
        float g;
        float b;
        float a;
    };
    class Palette
    {
    public:
        static const Color Black;
        static const Color LightBlack;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color LightYellow;
        static const Color LightBlue;
        static const Color LightPink;
        static const Color LightGreen;
    };

}