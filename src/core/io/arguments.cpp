#include "../../main_system.hpp"
#include <core/io/arguments.hpp>

namespace sinen {
int Arguments::getArgc() { return MainSystem::argc; }
std::vector<std::string> Arguments::getArgv() { return MainSystem::argv; }
} // namespace sinen