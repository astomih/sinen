#ifndef SINEN_STRING_HPP
#define SINEN_STRING_HPP
#include <core/allocator/global_allocator.hpp>
#include <string>
namespace sinen {
class String : public std::pmr::string {
public:
  String() : std::pmr::string(gA) {}
  String(Allocator *pA) : std::pmr::string(gA) {}
  String(const char *ptr, Allocator *pA) : std::pmr::string(ptr, pA) {}
};
} // namespace sinen

#endif // !SINEN_STRING_HPP