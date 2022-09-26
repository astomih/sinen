#include <io/file.hpp>
#include <io/json.hpp>
#include <rapidjson/document.h>

namespace sinen {
json::json() {}

json::~json() {}

bool json::read(const std::string_view &path) {
  file f;
  if (!f.open(path, file::mode::r))
    return false;
  void *buffer = malloc(f.size());
  f.read(buffer, f.size(), 1);
  std::string data = static_cast<char *>(buffer);
  f.close();
  rapidjson::Document doc;
  doc.Parse(data.c_str());
  if (doc.HasParseError())
    return false;
  return true;
}

bool json::write(const std::string_view &path) {}

} // namespace sinen