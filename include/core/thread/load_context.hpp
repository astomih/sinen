#ifndef SINEN_LOAD_CONTEXT_HPP
#define SINEN_LOAD_CONTEXT_HPP

#include <core/thread/task_group.hpp>

namespace sinen {

inline thread_local const TaskGroup *gLoadContext = nullptr;

class LoadContext {
public:
  static TaskGroup current() {
    return gLoadContext ? *gLoadContext : TaskGroup{};
  }

  static const TaskGroup *currentPtr() { return gLoadContext; }

  static void setCurrent(const TaskGroup *group) { gLoadContext = group; }
};

class ScopedLoadContext {
public:
  explicit ScopedLoadContext(const TaskGroup &group)
      : prev_(LoadContext::currentPtr()) {
    LoadContext::setCurrent(&group);
  }

  ~ScopedLoadContext() { LoadContext::setCurrent(prev_); }

  ScopedLoadContext(const ScopedLoadContext &) = delete;
  ScopedLoadContext &operator=(const ScopedLoadContext &) = delete;

private:
  const TaskGroup *prev_ = nullptr;
};

} // namespace sinen

#endif // SINEN_LOAD_CONTEXT_HPP
