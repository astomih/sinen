#include <Nen.hpp>
#include <wchar.h>

namespace nen {
effect::effect(std::string_view path) : loop(false) {}

effect::~effect() {}
} // namespace nen