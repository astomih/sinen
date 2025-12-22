// std libraries
#include <algorithm>
#include <cstdint>

// external libraries
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// internal libraries
#include "json.hpp"
#include <core/io/file.hpp>
#include <core/logger/logger.hpp>

namespace sinen {
class Json::Array::impl {
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
Json::Array::Array() {}
Json::Array::~Array() {
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
Json::Object Json::Array::operator[](std::size_t index) {
  Object obj;
  obj.pimpl = makePtr<Json::Object::Implements>(pimpl->value[index],
                                                this->pimpl->m_json);
  return obj;
}
void Json::Array::pushBack(Object &obj) {
  pimpl->value.PushBack(obj.pimpl->value,
                        pimpl->m_json->pimpl->doc.GetAllocator());
}
std::size_t Json::Array::size() const { return pimpl->value.Size(); }
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

Json::Object Json::Object::operator[](const StringView &key) {
  Object obj;
  obj.pimpl =
      makePtr<Implements>(pimpl->value[key.data()], this->pimpl->m_json);
  return obj;
}

std::int32_t Json::Object::getInt32() { return pimpl->value.GetInt(); }
std::uint32_t Json::Object::getUint32() { return pimpl->value.GetUint(); }
std::int64_t Json::Object::getInt64() { return pimpl->value.GetInt64(); }
std::uint64_t Json::Object::getUint64() { return pimpl->value.GetUint64(); }
float Json::Object::getFloat() { return pimpl->value.GetFloat(); }
double Json::Object::getDouble() { return pimpl->value.GetDouble(); }
String Json::Object::getString() { return pimpl->value.GetString(); }
bool Json::Object::getBool() { return pimpl->value.GetBool(); }

Json::Array Json::Object::getArray() {
  Array arr;
  arr.pimpl = makePtr<Json::Array::impl>(pimpl->value, pimpl->m_json);
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
void Json::Object::setString(StringView value) {
  pimpl->value.SetString(value.data(), value.size());
}
void Json::Object::setBool(bool value) { pimpl->value.SetBool(value); }

void Json::Object::setArray(Array &value) {
  pimpl->value.SetArray();
  for (std::size_t i = 0; i < value.size(); i++) {
    pimpl->value.PushBack(value[i].pimpl->value,
                          pimpl->m_json->pimpl->doc.GetAllocator());
  }
}

void Json::Object::addMember(StringView key, int value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(StringView key, float value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(StringView key, StringView value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), value.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, v, pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(StringView key, Object &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

void Json::Object::addMember(StringView key, Array &value) {
  rapidjson::Value k(key.data(), key.size(),
                     pimpl->m_json->pimpl->doc.GetAllocator());
  pimpl->value.AddMember(k, value.pimpl->value,
                         pimpl->m_json->pimpl->doc.GetAllocator());
}

Json::Json() : pimpl(makeUnique<Json::Implements>()) {}

Json::~Json() {}
void Json::parse(StringView str) {
  pimpl->doc.Parse(str.data());
  if (pimpl->doc.HasParseError()) {
    Logger::critical("%d", pimpl->doc.GetParseError());
  }
}
Json::Object Json::operator[](StringView key) {
  Object obj;
  if (pimpl->doc.HasMember(key.data())) {
    obj.pimpl = makePtr<Json::Object::Implements>(pimpl->doc[key.data()], this);
  }
  return obj;
}

void Json::addMember(StringView key, int value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(StringView key, float value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value);
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(StringView key, StringView value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  rapidjson::Value v(value.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, v, pimpl->doc.GetAllocator());
}

void Json::addMember(StringView key, Object &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}

void Json::addMember(StringView key, Array &value) {
  rapidjson::Value k(key.data(), pimpl->doc.GetAllocator());
  pimpl->doc.AddMember(k, value.pimpl->value, pimpl->doc.GetAllocator());
}
Json::Object Json::createObject() {
  Object obj;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kObjectType);
  pimpl->values.push_back(v);
  obj.pimpl = makePtr<Json::Object::Implements>(*v, this);
  return obj;
}
Json::Array Json::createArray() {
  Array arr;
  rapidjson::Value *v = new rapidjson::Value(rapidjson::kArrayType);
  pimpl->values.push_back(v);
  arr.pimpl = makePtr<Json::Array::impl>(*v, this);
  return arr;
}

String Json::toString() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  pimpl->doc.Accept(writer);
  return String(buffer.GetString());
}

std::size_t Json::size() { return pimpl->doc.Size(); }

} // namespace sinen
