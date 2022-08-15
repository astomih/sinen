#ifndef SINEN_DYNAMIC_HANDLER_HPP
#define SINEN_DYNAMIC_HANDLER_HPP
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <utility>

#include "handle_t.hpp"

namespace sinen {
template <class T, class DP = std::default_delete<T>> class handler {
public:
  handler() = default;
  handler(const handler &h) = default;
  handler(handler &&) = default;
  std::unordered_map<handle_t, std::unique_ptr<T, DP>> data;

  T &operator[](const handle_t &handle) {
    return *reinterpret_cast<T *>(data[handle].get());
  }
  template <class S = T, typename... Args> handle_t create(Args &&...__args) {
    handle_t handle = 0;
    while (data.contains(handle))
      ++handle;
    data.emplace(handle, std::make_unique<S>(std::forward<Args>(__args)...));
    return handle;
  }
  handle_t move(std::unique_ptr<T, DP> ptr) {
    handle_t h = 0;
    while (data.contains(h))
      ++h;
    data.emplace(h, std::move(ptr));
    return h;
  }
  void move(handle_t handle, std::unique_ptr<T, DP> ptr) {
    if (data.contains(handle)) {
      data.erase(handle);
    }
    data[handle] = std::move(ptr);
  }
  template <class S = T> S &get(const handle_t &handle) {
    return *reinterpret_cast<S *>(data[handle].get());
  }
  template <class S = T> S *get_raw(const handle_t &handle) {
    return reinterpret_cast<S *>(data[handle].get());
  }
  void clear() { data.clear(); }
  bool contains(const handle_t &handle) { return data.contains(handle); }
  void remove(const handle_t &handle) {
    auto ptr = std::move(data[handle]);
    data.erase(handle);
  }
};
} // namespace sinen
#endif // !SINEN_HANDLER_HPP
