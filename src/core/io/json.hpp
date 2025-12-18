#ifndef SINEN_JSON_HPP
#define SINEN_JSON_HPP
#include "file.hpp"
#include <memory>
#include <string_view>

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
    std::shared_ptr<impl> pimpl;
  };
  /**
   * @brief Json object class
   *
   */
  class Object {
    friend Json;

  public:
    ~Object();
    Object operator[](const std::string_view &key);
    std::int32_t getInt32();
    std::uint32_t getUint32();
    std::int64_t getInt64();
    std::uint64_t getUint64();
    float getFloat();
    double getDouble();
    std::string getString();
    bool getBool();
    Array getArray();

    void setInt32(std::int32_t value);
    void setUint32(std::uint32_t value);
    void setInt64(std::int64_t value);
    void setUint64(std::uint64_t value);
    void setFloat(float value);
    void setDouble(double value);
    void setString(std::string_view value);
    void setBool(bool value);
    void setArray(Array &value);

    void addMember(std::string_view key, int value);
    void addMember(std::string_view key, float value);
    void addMember(std::string_view key, std::string_view value);
    void addMember(std::string_view key, Object &value);
    void addMember(std::string_view key, Array &value);

  private:
    Object();
    class Implements;
    std::shared_ptr<Implements> pimpl;
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
  void parse(std::string_view str);

  void addMember(std::string_view key, int value);
  void addMember(std::string_view key, float value);
  void addMember(std::string_view key, std::string_view value);
  void addMember(std::string_view key, Object &value);
  void addMember(std::string_view key, Array &value);

  Object createObject();
  Array createArray();

  std::string toString();

  std::size_t size();

  Object operator[](std::string_view key);

private:
  friend Object;
  struct Implements;
  std::unique_ptr<Implements> pimpl;
};

} // namespace sinen
#endif