#include <Utility/Singleton.hpp>

namespace nen
{
	std::list<std::function<void(void)>> SingletonFinalizer::finalizers = std::list<std::function<void(void)>>();


	void SingletonFinalizer::AddFinalizer(const std::function<void(void)>& func)
	{
		finalizers.push_front(func);
	}

	void SingletonFinalizer::Finalize()
	{
		for (const auto& func : finalizers)
		{
			if (func)
				func();
		}
		finalizers.clear();
	}
}