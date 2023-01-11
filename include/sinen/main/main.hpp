#ifndef SINEN_MAIN_HPP
#define SINEN_MAIN_HPP
#include <string>
namespace sinen {
/**
 * @brief Engine main
 *
 */
class main {
public:
  static bool activate();
  static int run();
  static void change_scene(const std::string &scene_number);
  static std::string get_current_scene_number();

private:
  static bool deactivate();
};
} // namespace sinen
#endif // !SINEN_MAIN_HPP