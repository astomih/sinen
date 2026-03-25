#ifndef SINEN_FUTURE_POLL_HPP
#define SINEN_FUTURE_POLL_HPP

#include <core/data/ptr.hpp>
#include <core/thread/task_group.hpp>

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <utility>

namespace sinen {

template <class State, class Schedule, class OnReady, class OnInvalid>
void scheduleFuturePoll(const Ptr<State> &state, const TaskGroup &group,
                        Schedule &&schedule, OnReady &&onReady,
                        OnInvalid &&onInvalid) {
  auto scheduleFn = std::forward<Schedule>(schedule);
  auto onReadyFn = std::forward<OnReady>(onReady);
  auto onInvalidFn = std::forward<OnInvalid>(onInvalid);

  auto poll = std::make_shared<std::function<void()>>();
  *poll = [poll, state, group, scheduleFn, onReadyFn,
           onInvalidFn]() mutable {
    if (!state->future.valid()) {
      onInvalidFn();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      scheduleFn(*poll);
      return;
    }

    state->future.get();
    onReadyFn();
    group.done();
  };

  scheduleFn(*poll);
}

template <class State, class Schedule, class OnReady>
void scheduleFuturePoll(const Ptr<State> &state, const TaskGroup &group,
                        Schedule &&schedule, OnReady &&onReady) {
  scheduleFuturePoll(state, group, std::forward<Schedule>(schedule),
                     std::forward<OnReady>(onReady), [] {});
}

} // namespace sinen

#endif // SINEN_FUTURE_POLL_HPP
