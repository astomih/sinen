#ifndef SINEN_MAIN_HPP
#define SINEN_MAIN_HPP
#include <string>
namespace sinen {
#ifdef main
#undef main
#endif
class main {
public:
  static int activate();
  static void change_scene(const std::string &scene_number);
  static std::string get_current_scene_number();
};
} // namespace sinen
#endif // !SINEN_MAIN_HPP