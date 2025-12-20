#ifndef SINEN_PTR_HPP
#define SINEN_PTR_HPP
#include "core/allocator/global_allocator.hpp"
#include <core/allocator/allocator.hpp>
#include <cstddef>
#include <functional>
#include <memory>

namespace sinen {
template <typename T, typename A> struct Deleter {
  constexpr Deleter(A *pA = nullptr) : pA(pA) {}

  template <typename U,
            typename std::enable_if<std::is_convertible<U *, T *>::value,
                                    std::nullptr_t>::type = nullptr>
  Deleter(const Deleter<U, A> &) noexcept {}

  void operator()(T *ptr) const {
    if (ptr) {
      ptr->~T();
      pA->deallocate(ptr, sizeof(T));
    }
  }
  A *pA;
};
template <class T> using Ptr = std::shared_ptr<T>;
template <class T> using UniquePtr = std::unique_ptr<T, Deleter<T, Allocator>>;
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

template <class T, class... Args>
UniquePtr<T> makeUnique(Allocator *pA, Args &&...args) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T(std::forward<Args>(args)...);
  return std::unique_ptr<T, Deleter<T, Allocator>>(obj,
                                                   Deleter<T, Allocator>(pA));
}
template <class T> UniquePtr<T> makeUnique(Allocator *pA) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T();
  return UniquePtr<T>(obj, Deleter<T, Allocator>(pA));
}
template <class T, class... Args> UniquePtr<T> makeUnique(Args &&...args) {
  void *mem = gA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T(std::forward<Args>(args)...);
  return UniquePtr<T>(obj, Deleter<T, Allocator>(gA));
}
template <class T, class D = Deleter<T, Allocator>> UniquePtr<T> makeUnique() {
  void *mem = gA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T();
  return UniquePtr<T>(obj, Deleter<T, Allocator>(gA));
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