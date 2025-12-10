#ifndef EMSCRIPTEN
#ifndef PARANOIXA_VMA_HPP
#define PARANOIXA_VMA_HPP
#include <graphics/paranoixa/paranoixa.hpp>
#ifdef PARANOIXA_BUILD_DEBUG
#define VMA_RECORDING_ENABLED 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#endif
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>
#endif // PARANOIXA_VMA_HPP
#endif // EMSCRIPTEN