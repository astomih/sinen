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

namespace detail {
template <class State, class Schedule, class OnReady, class OnInvalid>
struct FuturePollContext {
  FuturePollContext(Ptr<State> state, TaskGroup group, Schedule schedule,
                    OnReady onReady, OnInvalid onInvalid)
      : state(std::move(state)), group(std::move(group)),
        schedule(std::move(schedule)), onReady(std::move(onReady)),
        onInvalid(std::move(onInvalid)) {}

  Ptr<State> state;
  TaskGroup group;
  Schedule schedule;
  OnReady onReady;
  OnInvalid onInvalid;
};

template <class Context> void scheduleFuturePollStep(Ptr<Context> context) {
  context->schedule([context]() mutable {
    if (!context->state->future.valid()) {
      context->onInvalid();
      context->group.done();
      return;
    }

    if (context->state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      scheduleFuturePollStep(context);
      return;
    }

    context->state->future.get();
    context->onReady();
    context->group.done();
  });
}
} // namespace detail

template <class State, class Schedule, class OnReady, class OnInvalid>
void scheduleFuturePoll(const Ptr<State> &state, const TaskGroup &group,
                        Schedule &&schedule, OnReady &&onReady,
                        OnInvalid &&onInvalid) {
  using Context =
      detail::FuturePollContext<State, std::decay_t<Schedule>,
                                std::decay_t<OnReady>, std::decay_t<OnInvalid>>;
  auto context =
      makePtr<Context>(state, group, std::forward<Schedule>(schedule),
                       std::forward<OnReady>(onReady),
                       std::forward<OnInvalid>(onInvalid));
  detail::scheduleFuturePollStep(context);
}

template <class State, class Schedule, class OnReady>
void scheduleFuturePoll(const Ptr<State> &state, const TaskGroup &group,
                        Schedule &&schedule, OnReady &&onReady) {
  scheduleFuturePoll(state, group, std::forward<Schedule>(schedule),
                     std::forward<OnReady>(onReady), [] {});
}

} // namespace sinen

#endif // SINEN_FUTURE_POLL_HPP
