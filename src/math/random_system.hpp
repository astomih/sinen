#ifndef SINEN_RANDOM_SYSTEM_HPP
#define SINEN_RANDOM_SYSTEM_HPP
#include <random>

namespace sinen {
class RandomSystem {
public:
  static bool initialize();
  static void shutdown();

  static std::mt19937 sGenerator;
};
} // namespace sinen
#endif // !SINEN_RANDOM_SYSTEM_HPP
