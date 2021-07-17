#pragma once
#include "include/Component.h"
#include "include/AudioComponent.h"
#include "include/CircleComponent.h"
#include "include/Sprite2DComponent.h"
#include "include/Sprite3DComponent.h"
#include "include/FontComponent.hpp"
#include "include/InputComponent.h"
#include "include/MoveComponent.h"
#include "include/EffectComponent.hpp"
#ifndef EMSCRIPTEN
#include "include/ScriptComponent.h"
#endif
#ifdef NO_USE_NEN_NAMESPACE
using namespace nen;
#endif
