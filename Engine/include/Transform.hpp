#pragma once
#include "Math.hpp"
#include "Window.hpp"
#include <cstdio>
#include <string>
namespace nen
{
	class Transform
	{
	public:
		explicit Transform(class Actor& owner)
			:mOwner(owner)
		{}
		void LookAt(const Vector3f& target);
	private:
		Actor& mOwner;
	};
}
