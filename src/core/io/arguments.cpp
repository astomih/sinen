#include "../../main_system.hpp"
#include <core/io/arguments.hpp>

namespace sinen {
int Arguments::getArgc() { return MainSystem::argc; }
Array<String> Arguments::getArgv() { return MainSystem::argv; }
} // namespace sinen