#ifndef SINEN_VMA_HPP
#define SINEN_VMA_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#ifdef DEBUG
#define VMA_RECORDING_ENABLED 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#endif
#include <vk_mem_alloc.h>
#endif // !defined(EMSCRIPTEN) && !defined(ANDROID)
#endif // SINEN_VMA_HPP