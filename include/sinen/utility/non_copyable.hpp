#ifndef SINEN_NONCOPYABLE_HPP
#define SINEN_NONCOPYABLE_HPP

namespace sinen {
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

} // namespace sinen

#endif // !SINEN_NONCOPYABLE_HPP
