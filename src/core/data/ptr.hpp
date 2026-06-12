#ifndef SINEN_PTR_HPP
#define SINEN_PTR_HPP
#include "core/allocator/global_allocator.hpp"
#include <cassert>
#include <core/allocator/allocator.hpp>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace sinen {
template <typename T> struct Deleter {
  constexpr Deleter(Allocator *a = nullptr) noexcept
      : size(defaultSize()), alignment(defaultAlignment()), pA(a) {}
  constexpr Deleter(Allocator *a, size_t size) noexcept
      : size(size), alignment(defaultAlignment()), pA(a) {}
  constexpr Deleter(Allocator *a, size_t size, size_t alignment) noexcept
      : size(size), alignment(alignment), pA(a) {}
  template <typename U,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  Deleter(const Deleter<U> &other) noexcept
      : size(other.size), alignment(other.alignment), pA(other.pA) {}

  Deleter(const Deleter &) noexcept = default;
  Deleter &operator=(const Deleter &) noexcept = default;
  Deleter(Deleter &&) noexcept = default;
  Deleter &operator=(Deleter &&) noexcept = default;

  void operator()(T *ptr) const {
    if (!ptr)
      return;
    if constexpr (!std::is_void_v<T>) {
      ptr->~T();
    }
    assert(pA);
    pA->deallocate(ptr, size, alignment);
  }
  size_t size;
  size_t alignment;
  Allocator *pA;

private:
  static constexpr size_t defaultSize() noexcept {
    if constexpr (std::is_void_v<T>) {
      return 0;
    } else {
      return sizeof(T);
    }
  }

  static constexpr size_t defaultAlignment() noexcept {
    if constexpr (std::is_void_v<T>) {
      return alignof(std::max_align_t);
    } else {
      return alignof(T);
    }
  }
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
template <class T, class... Args>
[[deprecated("Pass Allocator* explicitly. Use makePtr(Allocator*, ...) "
             "instead.")]]
Ptr<T> makePtr(Args &&...args) {
  return std::allocate_shared<T>(
      std::pmr::polymorphic_allocator<T>(GlobalAllocator::get()),
      std::forward<Args>(args)...);
}

template <class T, class... Args>
UniquePtr<T> makeUnique(Allocator *pA, Args &&...args) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  try {
    T *obj = new (mem) T(std::forward<Args>(args)...);
    return UniquePtr<T>(obj, Deleter<T>(pA, sizeof(T), alignof(T)));
  } catch (...) {
    pA->deallocate(mem, sizeof(T), alignof(T));
    throw;
  }
}
template <class T> UniquePtr<T> makeUnique(Allocator *pA) {
  void *mem = pA->allocate(sizeof(T), alignof(T));
  try {
    T *obj = new (mem) T();
    return UniquePtr<T>(obj, Deleter<T>(pA, sizeof(T), alignof(T)));
  } catch (...) {
    pA->deallocate(mem, sizeof(T), alignof(T));
    throw;
  }
}
template <class T, class... Args>
[[deprecated("Pass Allocator* explicitly. Use makeUnique(Allocator*, ...) "
             "instead.")]]
UniquePtr<T> makeUnique(Args &&...args) {
  auto *allocator = GlobalAllocator::get();
  void *mem = allocator->allocate(sizeof(T), alignof(T));
  try {
    T *obj = new (mem) T(std::forward<Args>(args)...);
    return UniquePtr<T>(obj, Deleter<T>(allocator, sizeof(T), alignof(T)));
  } catch (...) {
    allocator->deallocate(mem, sizeof(T), alignof(T));
    throw;
  }
}
template <class T, class D = Deleter<T>>
[[deprecated("Pass Allocator* explicitly. Use makeUnique<T>(Allocator*) "
             "instead.")]]
UniquePtr<T> makeUnique() {
  return makeUnique<T>(GlobalAllocator::get());
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
