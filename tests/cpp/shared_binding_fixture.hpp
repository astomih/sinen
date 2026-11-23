#pragma once

// This definition is deliberately compiled into both the host and test DLLs.
namespace sinen::test {
struct SharedValue {
  explicit SharedValue(int value) : value(value) {}
  ~SharedValue() {
    if (destroyed)
      ++*destroyed;
  }
  int value;
  int *destroyed = nullptr;
};
} // namespace sinen::test
