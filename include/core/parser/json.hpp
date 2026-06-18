#ifndef SINEN_JSON_HPP
#define SINEN_JSON_HPP
#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>

namespace sinen {
/**
 * @brief Json read/write class
 *
 */
class Json {
public:
  class Object;
  /**
   * @brief Json array class
   *
   */
  class Array {
    friend Json;

  public:
    ~Array();
    Object operator[](std::size_t index);
    void pushBack(Object &obj);
    std::size_t size() const;

  private:
    Array();
    class impl;
    Ptr<impl> pimpl;
  };
  /**
   * @brief Json object class
   *
   */
  class Object {
    friend Json;

  public:
    ~Object();
    Object operator[](const StringView &key);
    std::int32_t getInt32();
    std::uint32_t getUint32();
    std::int64_t getInt64();
    std::uint64_t getUint64();
    float getFloat();
    double getDouble();
    String getString();
    bool getBool();
    Array getArray();

    void setInt32(std::int32_t value);
    void setUint32(std::uint32_t value);
    void setInt64(std::int64_t value);
    void setUint64(std::uint64_t value);
    void setFloat(float value);
    void setDouble(double value);
    void setString(StringView value);
    void setBool(bool value);
    void setArray(Array &value);

    void addMember(StringView key, int value);
    void addMember(StringView key, float value);
    void addMember(StringView key, StringView value);
    void addMember(StringView key, Object &value);
    void addMember(StringView key, Array &value);

  private:
    Object();
    class Implements;
    Ptr<Implements> pimpl;
  };
  /**
   * @brief Construct a new json object
   *
   */
  Json();
  /**
   * @brief Destroy the json object
   *
   */
  ~Json();
  /**
   * @brief Parse json string
   *
   * @param str
   */
  void parse(StringView str);

  void addMember(StringView key, int value);
  void addMember(StringView key, float value);
  void addMember(StringView key, StringView value);
  void addMember(StringView key, Object &value);
  void addMember(StringView key, Array &value);

  Object createObject();
  Array createArray();

  String toString();

  std::size_t size();

  Object operator[](StringView key);

private:
  friend Object;
  struct Implements;
  UniquePtr<Implements> pimpl;
};

} // namespace sinen
#endif