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
class json : public file {
public:
  /**
   * @brief Json object class
   *
   */
  class object {
    friend json;

  public:
    object operator[](const std::string_view &key);
    std::int32_t get_int32();
    std::uint32_t get_uint32();
    std::int64_t get_int64();
    std::uint64_t get_uint64();
    double get_double();
    std::string get_string();
    bool get_bool();

  private:
    object();
    class impl;
    std::unique_ptr<impl> pimpl;
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
   * @brief Read json file
   *
   * @param path
   * @return true
   * @return false
   */
  bool read(const std::string_view &path);
  /**
   * @brief Write json file
   *
   * @param path
   * @return true
   * @return false
   */
  bool write(const std::string_view &path);

  object operator[](const std::string_view &key);

private:
  friend object;
  struct impl;
  std::unique_ptr<impl> pimpl;
};

} // namespace sinen
#endif