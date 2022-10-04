// std libraries
#include <string>
#include <string_view>

// external libraries
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// internal libraries
#include <io/file.hpp>
#include <io/json.hpp>

namespace sinen {

class json::object::impl {
public:
  impl(rapidjson::Value &value) : value(value) {}
  rapidjson::Value &value;

private:
};
class json::impl {
public:
  rapidjson::Document doc;
};
json::object json::object::operator[](const std::string_view &key) {
  object obj;
  obj.pimpl = std::make_unique<impl>(pimpl->value[key.data()]);
  return obj;
}
std::int32_t json::object::get_int32() { return pimpl->value.GetInt(); }
std::uint32_t json::object::get_uint32() { return pimpl->value.GetUint(); }
std::int64_t json::object::get_int64() { return pimpl->value.GetInt64(); }
std::uint64_t json::object::get_uint64() { return pimpl->value.GetUint64(); }
double json::object::get_double() { return pimpl->value.GetDouble(); }
std::string json::object::get_string() {
  return std::string(pimpl->value.GetString());
}
bool json::object::get_bool() { return pimpl->value.GetBool(); }

json::json() {}

json::~json() {}

bool json::read(const std::string_view &path) {
  file f;
  if (!f.open(path, file::mode::r))
    return false;
  void *buffer = malloc(f.size());
  f.read(buffer, f.size(), 1);
  std::string data(static_cast<char *>(buffer));
  f.close();
  pimpl->doc.Parse(data.c_str());
  if (pimpl->doc.HasParseError())
    return false;
  return true;
}

bool json::write(const std::string_view &path) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  pimpl->doc.Accept(writer);
  file f;
  if (!f.open(path, file::mode::w))
    return false;
  f.write((void *)buffer.GetString(), buffer.GetSize(), 0);
  f.close();
  return true;
}

} // namespace sinen