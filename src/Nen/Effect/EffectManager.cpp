#include "EffectManager.h"
namespace nen
{
	EffectManager::EffectManager()
		: manager(Effekseer::Manager::Create(8000))
	{
	}
	Effekseer::EffectRef EffectManager::GetEffect(const std::u16string& filePath)
	{
		if (!effects.contains(filePath))
		{
			auto ref = Effekseer::Effect::Create(this->manager, filePath.c_str(), 10.f);
			effects.insert({ filePath, ref });
			return ref;
		}
		else
		{
			return effects.find(filePath)->second;
		}
	}

}
