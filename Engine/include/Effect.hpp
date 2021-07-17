#pragma once
namespace nen
{
	class Effect
	{
	private:
		Effect() = default;
	public:
		void Init(class VKBase* vkrenderer);
		bool Load(const std::u16string& filePath);
		void Remove();
		void Draw();
	};

}
