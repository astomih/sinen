#include <core/io/arguments.hpp>

namespace sinen {
int Arguments::getArgc() { return argc; }
Array<String> Arguments::getArgv() { return argv; }
} // namespace sinen