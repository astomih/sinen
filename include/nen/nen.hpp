#ifndef NEN_HPP
#define NEN_HPP
#ifdef ANDROID
#define MOBILE
#endif
#include "Math.hpp"
#include "Vertex.h"
#include "VertexArray.h"
#include "Random.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.hpp"
#include "Window.hpp"
#include "Transform.hpp"
#include "Logger.h"
#include "AudioSystem.h"
#include "AudioEvent.h"
#include "Renderer.h"
#include "Transition.h"
#include "InputSystem.h"
#include "Effect.hpp"
#include "Script.h"
#include "ObjLoader.h"
#include "scene/Scene.hpp"
#include "actor/Actors.hpp"
#include "component/Components.hpp"
#endif
#ifdef NO_USE_NEN_NAMESPACE
using namespace nen;
#endif
