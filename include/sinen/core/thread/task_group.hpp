#ifndef SINEN_TASK_GROUP_HPP
#define SINEN_TASK_GROUP_HPP

#include <core/data/ptr.hpp>

#include <atomic>
#include <cstdint>

namespace sinen {

class TaskGroup {
public:
  TaskGroup() = default;

  static TaskGroup create() { return TaskGroup(makePtr<State>()); }

  void add(uint32_t count = 1) const {
    if (!state_ || count == 0)
      return;
    state_->total.fetch_add(count, std::memory_order_relaxed);
    state_->pending.fetch_add(count, std::memory_order_relaxed);
  }

  void done(uint32_t count = 1) const {
    if (!state_ || count == 0)
      return;
    state_->pending.fetch_sub(count, std::memory_order_relaxed);
  }

  uint32_t pending() const {
    return state_ ? state_->pending.load(std::memory_order_relaxed) : 0;
  }

  uint32_t total() const {
    return state_ ? state_->total.load(std::memory_order_relaxed) : 0;
  }

  bool isDone() const { return pending() == 0; }

  explicit operator bool() const noexcept { return state_ != nullptr; }

private:
  struct State {
    std::atomic<uint32_t> pending{0};
    std::atomic<uint32_t> total{0};
  };

  explicit TaskGroup(Ptr<State> state) : state_(std::move(state)) {}

  Ptr<State> state_;
};

} // namespace sinen

#endif // SINEN_TASK_GROUP_HPP
