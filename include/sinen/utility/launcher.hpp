#ifndef SINEN_LAUNCHER_HPP
#define SINEN_LAUNCHER_HPP
#include <string>
namespace sinen {
bool initialize();
void launch();
void change_scene(std::string scene_number);
std::string get_current_scene_number();

} // namespace sinen
#endif // !SINEN_LAUNCHER_HPP
