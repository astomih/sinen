#include <core/data/table_string.hpp>

#include <format>

namespace sinen {
String convert(StringView name, const TablePair &p, bool isReturn) {
  String s;
  s = name.data();
  s += "{ ";
  if (isReturn) {
    s += "\n";
  }
  for (int i = 0; i < p.size(); i++) {
    auto &v = p[i];
    if (isReturn) {
      s += "\t";
    }
    s += v.first + " = " + v.second;
    if (i < p.size() - 1) {
      s += ", ";
    }
    if (isReturn) {
      s += "\n";
    }
  }
  s += " }";
  return s;
}

String toStringTrim(double value) {
  String s(std::format("{}", value));

  auto dot = s.find('.');
  if (dot == String::npos) {
    return s + ".0";
  }

  bool allZero = true;
  for (size_t i = dot + 1; i < s.size(); ++i) {
    if (s[i] != '0') {
      allZero = false;
      break;
    }
  }

  if (allZero) {
    return s.substr(0, dot + 2);
  }

  s.erase(s.find_last_not_of('0') + 1);
  return s;
}
} // namespace sinen
