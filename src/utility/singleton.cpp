#include <utility/singleton.hpp>

namespace sinen {
std::list<std::function<void(void)>> singleton_finalizer::finalizers =
    std::list<std::function<void(void)>>();

void singleton_finalizer::add(const std::function<void(void)> &func) {
  finalizers.push_front(func);
}

void singleton_finalizer::finalize() {
  for (const auto &func : finalizers) {
    if (func)
      func();
  }
  finalizers.clear();
}
} // namespace sinen
