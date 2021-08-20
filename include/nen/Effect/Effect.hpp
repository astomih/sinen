#pragma once
#include <Math/Vector3.hpp>
#include <string>

namespace nen
{
	class Effect
	{
	public:
		Effect(std::u16string_view path);
		const Vector3& GetPosition() { return this->position; }
		void SetPosition(const Vector3& position) { this->position = position; }
		const std::u16string& GetPath() { return path; }
		int handle;

	private:
		Vector3 position;
		std::u16string path;
	};
}