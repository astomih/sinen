#include <Actors.hpp>
#include <Scene.hpp>
#include <Components.hpp>
#include <Engine.hpp>
namespace nen
{
	PlaneActor::PlaneActor(std::shared_ptr<Scene> scene, const std::string&& path, bool isUseLightSource)
		:Actor(scene)
	{

	}
}