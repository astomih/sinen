#pragma once
#ifndef NEN_NONCOPYABLE_HPP
#define NEN_NONCOPYABLE_HPP

namespace nen {

class non_copyable {
protected:
  non_copyable() {}
  ~non_copyable() {}

private:
  non_copyable(non_copyable const &) = delete;
  non_copyable &operator=(non_copyable const &) = delete;
  non_copyable(non_copyable &&) = delete;
  non_copyable &operator=(non_copyable &&) = delete;
};

} // namespace nen

#endif
