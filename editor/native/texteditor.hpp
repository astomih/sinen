#ifndef SINEN_TEXTEDITOR_HPP
#define SINEN_TEXTEDITOR_HPP
#include <memory>
#include <string>
namespace sinen {
class texteditor {
public:
  static void display();
  static std::string get_text();
  static void set_text(const std::string &text);
  static void set_script_name(const std::string &path);

private:
  class impl;
  static std::unique_ptr<impl> pimpl;
  static std::string path;
};
} // namespace sinen
#endif // SINEN_TEXTEDITOR_HPP