#ifndef SINEN_RANDOM_SYSTEM_HPP
#define SINEN_RANDOM_SYSTEM_HPP
#include <random>

namespace sinen {
class random_system {
public:
  void init();

  std::mt19937 sGenerator;
};
} // namespace sinen
#endif // !SINEN_RANDOM_SYSTEM_HPP