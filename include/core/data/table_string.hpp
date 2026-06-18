#ifndef SINEN_TABLE_STRING_HPP
#define SINEN_TABLE_STRING_HPP

#include <core/data/array.hpp>
#include <core/data/string.hpp>

#include <utility>

namespace sinen {
using TablePair = Array<std::pair<String, String>>;

String convert(StringView name, const TablePair &p, bool isReturn);
String toStringTrim(double value);
} // namespace sinen

#endif // SINEN_TABLE_STRING_HPP
