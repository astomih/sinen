#ifndef SINEN_PTR_HPP
#define SINEN_PTR_HPP
#include "core/allocator/global_allocator.hpp"
#include <core/allocator/allocator.hpp>
#include <memory>

namespace sinen {
template <class T> using Ptr = std::shared_ptr<T>;
template <class T> using Ref = std::weak_ptr<T>;

// Allocation wrapper functions
template <class T, class... Args>
Ptr<T> makePtr(Allocator *allocator, Args &&...args) {
  return std::allocate_shared<T>(std::pmr::polymorphic_allocator<T>(allocator),
                                 std::forward<Args>(args)...);
}
template <class T, class... Args> Ptr<T> makePtr(Args &&...args) {
  return std::allocate_shared<T>(std::pmr::polymorphic_allocator<T>(gA),
                                 std::forward<Args>(args)...);
}
template <class T, class U> Ptr<T> downCast(Ptr<U> ptr) {
#ifdef _DEBUG
  return std::dynamic_pointer_cast<T>(ptr);
#else
  return std::static_pointer_cast<T>(ptr);
#endif
}
} // namespace sinen

#endif