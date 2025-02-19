// std libraries
#include <algorithm>
#include <cstdint>
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
#include <logger/logger.hpp>

namespace sinen {
class Json::array::impl {
public:
  impl(rapidjson::Value &value, Json *_json) : value(value), m_json(_json) {}
  bool is_created_value = false;
  rapidjson::Value &value;
  Json *m_json;
};
class Json::object::impl {
public:
  impl(rapidjson::Value &value, Json *_json) : value(value), m_json(_json) {}
  bool is_created_value = false;
  rapidjson::Value &value;
  Json *m_json;

private:
};
class Json::impl {
public:
  rapidjson::Document doc;
  std::list<rapidjson::Value *> values;
};
Json::array::array() {}
Json::array::~array() {
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
Json::object Json::array::operator[](std::size_t index) {
  object obj;
  obj.pimpl = std::make_shared<Json::object::impl>(pimpl->value[index],
                                                   this->pimpl->m_json);
  return obj;
}
void Json::array::push_back(object &obj) {
  pimpl->value.PushBack(obj.pimpl->value,
                        pimpl->m_json->pimpl->doc.GetAllocator());
}
std::size_t Json::array::size() const { return pimpl->value.Size(); }
Json::object::object() {}
Json::object::~object() {
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

Json::object Json::object::operator[](const std::string_view &key) {
  object obj;
  obj.pimpl =
      std::make_shared<impl>(pimpl->value[key.data()], this->pimpl->m_json);
  return obj;
}

std::int32_t Json::object::get_int32() { return pimpl->value.GetInt(); }
std::uint32_t Json::object::get_uint32() { return pimpl->value.GetUint(); }
std::int64_t Json::object::get_int64() { return pimpl->value.GetInt64(); }
std::uint64_t Json::object::get_uint64() { return pimpl->value.GetUint64(); }
float Json::object::get_float() { return pimpl->value.GetFloat(); }
double Json::object::get_double() { return pimpl->value.GetDouble(); }
std::string Json::object::get_string() {
  return std::string(pimpl->value.GetString());
}
bool Json::object::get_bool() { return pimpl->value.GetBool(); }

Json::array Json::object::get_array() {
  array arr;
  arr.pimpl = std::make_shared<Json::array::impl>(pimpl->value, pimpl->m_json);
  return arr;
}

void Json::object::set_int32(std::int32_t value) { pimpl->value.SetInt(value); }
void Json::object::set_uint32(std::uint32_t value) {
  pimpl->value.SetUint(value);
}
void Json::object::set_int64(std::int64_t value) {
  pimpl->value.SetInt64(value);
}
void Json::object::set_uint64(std::uint64_t value) {
  pimpl->value.SetUint64(value);
}
void Json::object::set_double(double value) { pimpl->value.SetDouble(value); }
void Json::object::set_float(float value) { pimpl->value.SetFloat(value); }
void Json::object::set_string(std::string_view value) {
  pimpl->value.SetString(value.data(), value.size());
}
void Json::object::set_bool(bool value) { pimpl->value.SetBool(value); }

void Json::object::set_array(array &value) {
  pimpl->value.SetArray();
  for (std::size_t i = 0; i < value.size(); i++) {
    pimpl->value.PushBack(value[i].pimpl->value,
                          pimpl->m_json->pimpl->doc.GetAllocator());
  }
}

void Json::object::add_member(std::string_view key, int value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::object::add_member(std::string_view key, float value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::object::add_member(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), value.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::object::add_member(std::string_view key, object &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::object::add_member(std::string_view key, array &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

Json::Json() : pimpl(std::make_unique<Json::impl>()) {}

Json::~Json() {}
void Json::parse(std::string_view str) {
  pimpl->doc.Parse(str.data());
  if (pimpl->doc.HasParseError()) {
    Logger::critical("%d", pimpl->doc.GetParseError());
  }
}
Json::object Json::operator[](std::string_view key) {
  object obj;
  if (pimpl->doc.HasMember(key.data())) {
    obj.pimpl =
        std::make_shared<Json::object::impl>(pimpl->doc[key.data()], this);
  }
  return obj;
}

void Json::add_member(std::string_view key, int value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::add_member(std::string_view key, float value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::add_member(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::add_member(std::string_view key, object &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}

void Json::add_member(std::string_view key, array &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}
Json::object Json::create_object() {
  object obj;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kObjectType);
  pimpl->values.push_back(v);
  obj.pimpl = std::make_shared<Json::object::impl>(*v, this);
  return obj;
}
Json::array Json::create_array() {
  array arr;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kArrayType);
  pimpl->values.push_back(v);
  arr.pimpl = std::make_shared<Json::array::impl>(*v, this);
  return arr;
}

std::string Json::to_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  pimpl->doc.Accept(writer);
  return buffer.GetString();
}

std::size_t Json::size() { return pimpl->doc.Size(); }

} // namespace sinen
