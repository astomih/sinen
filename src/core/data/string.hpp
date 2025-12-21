#ifndef SINEN_STRING_HPP
#define SINEN_STRING_HPP
#include <string>
#include <string_view>
namespace sinen {
using String = std::pmr::string;
using StringView = std::string_view;
} // namespace sinen

#endif // !SINEN_STRING_HPP