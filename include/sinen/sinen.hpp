#ifndef SINEN_HPP
#define SINEN_HPP

// RHI
#include <paranoixa/paranoixa.hpp>

// Asset
#include "asset/asset.hpp"

// Core
#include "core/core.hpp"

// Graphics
#include "graphics/graphics.hpp"

// Logic
#include "logic/logic.hpp"

// Math
#include "math/math.hpp"

// Platform
#include "platform/platform.hpp"

namespace sinen {
class Sinen {
public:
  /**
   * @brief Initialize sinen engine
   *
   * @return true Success to initialize
   * @return false Failed to initialize
   */
  static bool initialize(int argc, char *argv[]);
  /**
   * @brief Run sinen engine
   *
   */
  static void run();
  /**
   * @brief Shutdown sinen engine
   */
  static void shutdown();
};
} // namespace sinen

#endif

#ifdef NO_USE_SINEN_NAMESPACE
using namespace sinen;
#endif
