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
#include <core/io/file.hpp>
#include <core/io/json.hpp>
#include <core/logger/logger.hpp>

namespace sinen {
class Json::array::impl {
public:
  impl(rapidjson::Value &value, Json *_json) : value(value), m_json(_json) {}
  bool is_created_value = false;
  rapidjson::Value &value;
  Json *m_json;
};
class Json::Object::Implements {
public:
  Implements(rapidjson::Value &value, Json *_json)
      : value(value), m_json(_json) {}
  bool is_created_value = false;
  rapidjson::Value &value;
  Json *m_json;

private:
};
class Json::Implements {
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
Json::Object Json::array::operator[](std::size_t index) {
  Object obj;
  obj.pimpl = std::make_shared<Json::Object::Implements>(pimpl->value[index],
                                                         this->pimpl->m_json);
  return obj;
}
void Json::array::push_back(Object &obj) {
  pimpl->value.PushBack(obj.pimpl->value,
                        pimpl->m_json->pimpl->doc.GetAllocator());
}
std::size_t Json::array::size() const { return pimpl->value.Size(); }
Json::Object::Object() {}
Json::Object::~Object() {
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

Json::Object Json::Object::operator[](const std::string_view &key) {
  Object obj;
  obj.pimpl = std::make_shared<Implements>(pimpl->value[key.data()],
                                           this->pimpl->m_json);
  return obj;
}

std::int32_t Json::Object::getInt32() { return pimpl->value.GetInt(); }
std::uint32_t Json::Object::getUint32() { return pimpl->value.GetUint(); }
std::int64_t Json::Object::getInt64() { return pimpl->value.GetInt64(); }
std::uint64_t Json::Object::getUint64() { return pimpl->value.GetUint64(); }
float Json::Object::getFloat() { return pimpl->value.GetFloat(); }
double Json::Object::getDouble() { return pimpl->value.GetDouble(); }
std::string Json::Object::getString() {
  return std::string(pimpl->value.GetString());
}
bool Json::Object::getBool() { return pimpl->value.GetBool(); }

Json::array Json::Object::getArray() {
  array arr;
  arr.pimpl = std::make_shared<Json::array::impl>(pimpl->value, pimpl->m_json);
  return arr;
}

void Json::Object::setInt32(std::int32_t value) { pimpl->value.SetInt(value); }
void Json::Object::setUint32(std::uint32_t value) {
  pimpl->value.SetUint(value);
}
void Json::Object::setInt64(std::int64_t value) {
  pimpl->value.SetInt64(value);
}
void Json::Object::setUint64(std::uint64_t value) {
  pimpl->value.SetUint64(value);
}
void Json::Object::setDouble(double value) { pimpl->value.SetDouble(value); }
void Json::Object::setFloat(float value) { pimpl->value.SetFloat(value); }
void Json::Object::setString(std::string_view value) {
  pimpl->value.SetString(value.data(), value.size());
}
void Json::Object::setBool(bool value) { pimpl->value.SetBool(value); }

void Json::Object::setArray(array &value) {
  pimpl->value.SetArray();
  for (std::size_t i = 0; i < value.size(); i++) {
    pimpl->value.PushBack(value[i].pimpl->value,
                          pimpl->m_json->pimpl->doc.GetAllocator());
  }
}

void Json::Object::addMember(std::string_view key, int value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(std::string_view key, float value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), value.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(std::string_view key, Object &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(std::string_view key, array &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

Json::Json() : pimpl(std::make_unique<Json::Implements>()) {}

Json::~Json() {}
void Json::parse(std::string_view str) {
  pimpl->doc.Parse(str.data());
  if (pimpl->doc.HasParseError()) {
    Logger::critical("%d", pimpl->doc.GetParseError());
  }
}
Json::Object Json::operator[](std::string_view key) {
  Object obj;
  if (pimpl->doc.HasMember(key.data())) {
    obj.pimpl = std::make_shared<Json::Object::Implements>(
        pimpl->doc[key.data()], this);
  }
  return obj;
}

void Json::addMember(std::string_view key, int value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(std::string_view key, float value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(std::string_view key, std::string_view value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(std::string_view key, Object &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}

void Json::addMember(std::string_view key, array &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}
Json::Object Json::createObject() {
  Object obj;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kObjectType);
  pimpl->values.push_back(v);
  obj.pimpl = std::make_shared<Json::Object::Implements>(*v, this);
  return obj;
}
Json::array Json::createArray() {
  array arr;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kArrayType);
  pimpl->values.push_back(v);
  arr.pimpl = std::make_shared<Json::array::impl>(*v, this);
  return arr;
}

std::string Json::toString() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  pimpl->doc.Accept(writer);
  return buffer.GetString();
}

std::size_t Json::size() { return pimpl->doc.Size(); }

} // namespace sinen
