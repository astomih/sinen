#ifndef NEN_STATIC_HANDLER_HPP
#define NEN_STATIC_HANDLER_HPP
#include <functional>
#include <memory>
#include <unordered_map>

#include "handle_t.hpp"

namespace nen {
template <class T> class static_handler {
public:
  static_handler() = default;
  static_handler(const static_handler &) = delete;
  static_handler(static_handler &&) = default;
  std::unordered_map<handle_t, T> data;

  T &operator[](const handle_t &handle) { return data[handle]; }
  void add(handle_t &handle, const T &&data) {
    while (data.contains(handle))
      ++handle;
    data.emplace(handle, std::forward(data));
  }
  T &get(const handle_t &handle) { return data[handle]; }
  void clear() { data.clear(); }
  bool contains(const handle_t &handle) { return data.contains(handle); }
  void remove(const handle_t &handle) { data.erase(handle); }

private:
};
} // namespace nen
#endif