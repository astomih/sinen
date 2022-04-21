#include <utility/singleton.hpp>

namespace nen {
std::list<std::function<void(void)>> singleton_finalizer::finalizers =
    std::list<std::function<void(void)>>();

void singleton_finalizer::AddFinalizer(const std::function<void(void)> &func) {
  finalizers.push_front(func);
}

void singleton_finalizer::Finalize() {
  for (const auto &func : finalizers) {
    if (func)
      func();
  }
  finalizers.clear();
}
} // namespace nen