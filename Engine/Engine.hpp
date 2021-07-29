#ifndef NEN_ENGINE_HPP
#define NEN_ENGINE_HPP
#include "include/Math.hpp"
#include "include/Vertex.h"
#include "include/VertexArray.h"
#include "include/Vulkan/VKRenderer.h"
#include "include/Vulkan/Swapchain.h"
#include "include/Vulkan/VulkanShader.h"
#include "include/Vulkan/Pipeline.h"
#include "include/Vulkan/PipelineLayout.h"
#include "include/OpenGL/ShaderGL.h"
#include "include/OpenGL/GLRenderer.h"
#include "include/ES/ESRenderer.h"
#include "include/Random.h"
#include "include/Texture.h"
#include "include/Font.hpp"
#include "include/Window.hpp"
#include "include/Transform.hpp"
#include "include/DebugLog.hpp"
#include "include/AudioSystem.h"
#include "include/AudioEvent.h"
#include "include/Renderer.h"
#include "include/Transition.h"
#include "include/InputSystem.h"
#include "include/Effect.hpp"
#include "include/Script.h"
#include "include/ObjLoader.h"
#include "include/Vulkan/VulkanUtil.h"
#endif
#ifdef NO_USE_NEN_NAMESPACE
using namespace nen;
#endif
