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
class json {
public:
  class object;
  /**
   * @brief Json array class
   *
   */
  class array {
    friend json;

  public:
    ~array();
    object operator[](std::size_t index);
    void push_back(object &obj);
    const std::size_t size() const;

  private:
    array();
    class impl;
    std::shared_ptr<impl> pimpl;
  };
  /**
   * @brief Json object class
   *
   */
  class object {
    friend json;

  public:
    ~object();
    object operator[](const std::string_view &key);
    std::int32_t get_int32();
    std::uint32_t get_uint32();
    std::int64_t get_int64();
    std::uint64_t get_uint64();
    float get_float();
    double get_double();
    std::string get_string();
    bool get_bool();
    array get_array();

    void set_int32(std::int32_t value);
    void set_uint32(std::uint32_t value);
    void set_int64(std::int64_t value);
    void set_uint64(std::uint64_t value);
    void set_float(float value);
    void set_double(double value);
    void set_string(std::string_view value);
    void set_bool(bool value);
    void set_array(array &value);

    void add_member(std::string_view key, int value);
    void add_member(std::string_view key, float value);
    void add_member(std::string_view key, std::string_view value);
    void add_member(std::string_view key, object &value);
    void add_member(std::string_view key, array &value);

  private:
    object();
    class impl;
    std::shared_ptr<impl> pimpl;
  };
  /**
   * @brief Construct a new json object
   *
   */
  json();
  /**
   * @brief Destroy the json object
   *
   */
  ~json();
  /**
   * @brief Parse json string
   *
   * @param str
   */
  void parse(std::string_view str);

  void add_member(std::string_view key, int value);
  void add_member(std::string_view key, float value);
  void add_member(std::string_view key, std::string_view value);
  void add_member(std::string_view key, object &value);
  void add_member(std::string_view key, array &value);

  object create_object();
  array create_array();

  std::string to_string();

  std::size_t size();

  object operator[](std::string_view key);

private:
  friend object;
  struct impl;
  std::unique_ptr<impl> pimpl;
};

} // namespace sinen
#endif