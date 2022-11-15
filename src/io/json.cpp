// std libraries
#include <algorithm>
#include <list>
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
  impl(rapidjson::Value &value, json *_json) : value(value), m_json(_json) {}
  bool is_created_value = false;
  rapidjson::Value &value;
  json *m_json;

private:
};
class json::impl {
public:
  rapidjson::Document doc;
  std::list<rapidjson::Value *> values;
};

json::object::object() {}
json::object::~object() {
  if (pimpl->is_created_value) {
    auto itr = std::find(pimpl->m_json->pimpl->values.begin(),
                         pimpl->m_json->pimpl->values.end(), &pimpl->value);
    if (itr != pimpl->m_json->pimpl->values.end()) {
      rapidjson::Value *v = *itr;
      pimpl->m_json->pimpl->values.erase(itr);
      delete v;
    }
  }
}

json::object json::object::operator[](const std::string_view &key) {
  object obj;
  obj.pimpl =
      std::make_shared<impl>(pimpl->value[key.data()], this->pimpl->m_json);
  return obj;
}

std::int32_t json::object::get_int32() { return pimpl->value.GetInt(); }
std::uint32_t json::object::get_uint32() { return pimpl->value.GetUint(); }
std::int64_t json::object::get_int64() { return pimpl->value.GetInt64(); }
std::uint64_t json::object::get_uint64() { return pimpl->value.GetUint64(); }
float json::object::get_float() { return pimpl->value.GetFloat(); }
double json::object::get_double() { return pimpl->value.GetDouble(); }
std::string json::object::get_string() {
  return std::string(pimpl->value.GetString());
}
bool json::object::get_bool() { return pimpl->value.GetBool(); }

void json::object::set_int32(std::int32_t value) { pimpl->value.SetInt(value); }
void json::object::set_uint32(std::uint32_t value) {
  pimpl->value.SetUint(value);
}
void json::object::set_int64(std::int64_t value) {
  pimpl->value.SetInt64(value);
}
void json::object::set_uint64(std::uint64_t value) {
  pimpl->value.SetUint64(value);
}
void json::object::set_double(double value) { pimpl->value.SetDouble(value); }
void json::object::set_float(float value) { pimpl->value.SetFloat(value); }
void json::object::set_string(std::string_view value) {
  pimpl->value.SetString(value.data(), value.size());
}
void json::object::set_bool(bool value) { pimpl->value.SetBool(value); }

void json::object::add_member(std::string_view key, int value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void json::object::add_member(std::string_view key, float value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void json::object::add_member(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), value.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void json::object::add_member(std::string_view key, object &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

json::json() : pimpl(std::make_unique<json::impl>()) {}

json::~json() {}

void json::parse(std::string_view str) { pimpl->doc.Parse(str.data()); }
json::object json::operator[](std::string_view key) {
  object obj;
  obj.pimpl =
      std::make_shared<json::object::impl>(pimpl->doc[key.data()], this);
  return obj;
}

void json::add_member(std::string_view key, int value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void json::add_member(std::string_view key, float value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void json::add_member(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void json::add_member(std::string_view key, object &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}

json::object json::create_object() {
  object obj;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kObjectType);
  pimpl->values.push_back(v);
  obj.pimpl = std::make_shared<json::object::impl>(*v, this);
  return obj;
}

std::string json::to_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  pimpl->doc.Accept(writer);
  return buffer.GetString();
}

} // namespace sinen