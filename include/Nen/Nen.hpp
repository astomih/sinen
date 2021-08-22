#ifndef NEN_HPP
#define NEN_HPP
#ifdef ANDROID
#define MOBILE
#endif
#include "Math/Math.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Transform.hpp"
#include "Math/Random.hpp"
#include "Actor/Actor.hpp"
#include "Actor/Text.hpp"
#include "Audio/AudioEvent.hpp"
#include "Audio/AudioSystem.hpp"
#include "Component/Component.hpp"
#include "Component/AudioComponent.hpp"
#include "Component/CircleComponent.hpp"
#include "Component/EffectComponent.hpp"
#include "Component/FontComponent.hpp"
#include "Component/InputComponent.hpp"
#include "Component/MoveComponent.hpp"
#include "Component/ScriptComponent.hpp"
#include "Component/Sprite2DComponent.hpp"
#include "Component/Sprite3DComponent.hpp"
#include "Effect/Effect.hpp"
#include "Font/Font.hpp"
#include "Input/InputSystem.hpp"
#include "Logger/Logger.hpp"
#include "Mesh/ObjLoader.hpp"
#include "Render/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "Sprite/Sprite.hpp"
#include "Script/Script.hpp"
#include "Texture/Texture.hpp"
#include "Vertex/Vertex.hpp"
#include "Vertex/VertexArray.hpp"
#include "Window/Window.hpp"
#endif
#ifdef NO_USE_NEN_NAMESPACE
using namespace nen;
#endif