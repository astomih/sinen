#ifndef SINEN_THREAD_POOL_HPP
#define SINEN_THREAD_POOL_HPP
#include <core/allocator/global_allocator.hpp>
#include <core/data/array.hpp>
#include <core/data/queue.hpp>

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

namespace sinen {

class ThreadPool {
public:
  explicit ThreadPool(std::size_t thread_count) {
    assert(thread_count > 0);
    workers_.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i) {
      workers_.emplace_back([this] { workerLoop(); });
    }
  }

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ~ThreadPool() { shutdown(); }

  template <class F, class... Args>
  auto submit(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<R> fut = task->get_future();
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (stopping_) {
        throw std::runtime_error("ThreadPool: submit on stopped pool");
      }
      tasks_.emplace([task]() { (*task)(); });
    }
    cv_.notify_one();
    return fut;
  }

  void shutdown() noexcept {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (stopping_)
        return;
      stopping_ = true;
    }
    cv_.notify_all();
    for (auto &t : workers_) {
      if (t.joinable())
        t.join();
    }
    workers_.clear();
  }

private:
  void workerLoop() {
    while (true) {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return stopping_ || !tasks_.empty(); });

        if (stopping_ && tasks_.empty()) {
          return; // terminate
        }
        job = std::move(tasks_.front());
        tasks_.pop();
      }
      // Run at out of lock
      job();
    }
  }

  std::mutex mtx_;
  std::condition_variable cv_;
  Queue<std::function<void()>> tasks_;
  Array<std::thread> workers_;
  bool stopping_ = false;
};
} // namespace sinen

#endif // SINEN_THREAD_POOL_HPP