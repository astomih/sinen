#ifndef SINEN_ASSERT_HPP
#define SINEN_ASSERT_HPP

namespace sinen {
class Assert {
public:
  static void report(bool condition, const char *cond) noexcept;
};

#define SN_ASSERT(condition) Assert::report(condition, #condition)
} // namespace sinen

#endif // SINEN_ASSERT_HPP