#ifndef SINEN_HPP
#define SINEN_HPP

// RHI
#include <paranoixa/paranoixa.hpp>

// Asset
#include "asset/asset.hpp"

// Core
#include "core/core.hpp"

// Logic
#include "logic/logic.hpp"

// Math
#include "math/math.hpp"

// Platform
#include "platform/platform.hpp"

// Render
#include "render/render.hpp"

namespace sinen {
/**
 * @brief Initialize sinen engine
 *
 * @return true Success to initialize
 * @return false Failed to initialize
 */
bool Initialize(int argc, char *argv[]);
/**
 * @brief Run sinen engine
 *
 */
void Run();
/**
 * @brief Shutdown sinen engine
 *
 * @return true Success to shutdown
 * @return false Failed to shutdown
 */
bool Shutdown();
} // namespace sinen

#endif

#ifdef NO_USE_SINEN_NAMESPACE
using namespace sinen;
#endif
