#pragma once
#include "Math.hpp"
#include <string>

namespace nen
{
	class Effect
	{
	public:
		Effect(std::u16string_view path);
		const Vector3f& GetPosition() { return this->position; }
		void SetPosition(const Vector3f& position) { this->position = position; }
		const std::u16string& GetPath() { return path; }
		int handle;

	private:
		Vector3f position;
		std::u16string path;
	};
}