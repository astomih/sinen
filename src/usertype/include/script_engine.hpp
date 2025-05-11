#include <pocketpy.h>
#include <pybind11/operators.h>
#include <pybind11/pkbind.h>
#include <sol/sol.hpp>


namespace py = pybind11;
namespace sinen {
class script_engine {
public:
  static bool initialize(sol::state &v);
};
} // namespace sinen
