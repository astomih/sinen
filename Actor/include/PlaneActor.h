#pragma once
#include "Actor.h"

class PlaneActor : public Actor
{
public:
	PlaneActor(std::shared_ptr<class Scene> game,const std::string&& path , bool isUseLightSource=false);
private:
};