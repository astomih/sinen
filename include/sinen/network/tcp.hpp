#ifndef SINEN_TCP_HPP
#define SINEN_TCP_HPP
#include <cstdint>
#include <memory>
#include <string>

namespace sinen {
/**
 * @brief TCP client class
 *
 */
class tcp {
public:
  /**
   * @brief Construct a new tcp client object
   *
   */
  tcp();
  /**
   * @brief Destroy the tcp client object
   *
   */
  ~tcp();
  /**
   * @brief Resolve host
   *
   * @param address address
   * @param port port
   * @return true success
   * @return false failed
   */
  bool resolve_host(std::string_view address, uint16_t port);
  /**
   * @brief Connect to server
   *
   * @return true success
   * @return false failed
   */
  bool open();
  /**
   * @brief Close connection
   *
   */
  void close();
  /**
   * @brief Receive data
   *
   * @param data data ptr
   * @param max_length data max length
   * @return true Success
   * @return false Failed
   */
  bool receive(void *data, int max_length);
  /**
   * @brief Send data
   *
   * @param data data ptr
   * @param size data size
   * @return true Success
   * @return false Failed
   */
  bool send(const void *data, int size);

private:
  class impl;
  std::unique_ptr<impl> m_impl;
};
} // namespace sinen
#endif // !SINEN_TCP_CLIENT_HPP
