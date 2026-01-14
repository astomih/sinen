#ifndef SINEN_PTR_HPP
#define SINEN_PTR_HPP
#include "core/allocator/global_allocator.hpp"
#include <cassert>
#include <core/allocator/allocator.hpp>
#include <cstddef>
#include <memory>

namespace sinen {
template <typename T> struct Deleter {
  constexpr Deleter(Allocator *a = nullptr, size_t size = sizeof(T)) noexcept
      : pA(a), size(size) {}
  template <typename U,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  Deleter(const Deleter<U> &other) noexcept : pA(other.pA), size(other.size) {}

  Deleter(Deleter &&) noexcept = default;
  Deleter &operator=(Deleter &&) noexcept = default;

  void operator()(T *ptr) const {
    if (!ptr)
      return;
    ptr->~T();
    assert(pA);
    pA->deallocate(ptr, size);
  }
  size_t size;
  Allocator *pA;
};
template <class T> using Ptr = std::shared_ptr<T>;
template <class T> using UniquePtr = std::unique_ptr<T, Deleter<T>>;
template <class T> using Ref = std::weak_ptr<T>;

// Allocation wrapper functions
template <class T, class... Args>
Ptr<T> makePtr(Allocator *allocator, Args &&...args) {
  return std::allocate_shared<T>(std::pmr::polymorphic_allocator<T>(allocator),
                                 std::forward<Args>(args)...);
}
template <class T, class... Args> Ptr<T> makePtr(Args &&...args) {
  return std::allocate_shared<T>(
      std::pmr::polymorphic_allocator<T>(GlobalAllocator::get()),
      std::forward<Args>(args)...);
}

template <class T, class... Args>
UniquePtr<T> makeUnique(Allocator *pA, Args &&...args) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T(std::forward<Args>(args)...);
  return std::unique_ptr<T, Deleter<T>>(obj, Deleter<T>(pA));
}
template <class T> UniquePtr<T> makeUnique(Allocator *pA) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T();
  return UniquePtr<T>(obj, Deleter<T>(pA));
}
template <class T, class... Args> UniquePtr<T> makeUnique(Args &&...args) {
  void *mem = GlobalAllocator::get()->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T(std::forward<Args>(args)...);
  return UniquePtr<T>(obj, Deleter<T>(GlobalAllocator::get()));
}
template <class T, class D = Deleter<T>> UniquePtr<T> makeUnique() {
  void *mem = GlobalAllocator::get()->allocate(sizeof(T), alignof(T));
  T *obj = new (mem) T();
  return UniquePtr<T>(obj, Deleter<T>(GlobalAllocator::get()));
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