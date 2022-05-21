#pragma once
#include <string>
namespace nen {
bool initialize();
void launch();
void change_scene(std::string scene_number);
std::string get_current_scene_number();

} // namespace nen