#pragma once
#include "Math.hpp"
#include "../Window/Window.hpp"
#include "../Actor/Actor.hpp"
#include <cstdio>
#include <string>
namespace nen
{
	class Transform
	{
	public:
		explicit Transform(class Actor &owner)
			: mOwner(owner)
		{
		}
		void LookAt(const Vector3 &target, const Vector3 &norm);

	private:
		Actor &mOwner;
	};
}
