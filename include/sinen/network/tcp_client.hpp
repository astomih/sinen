#ifndef SINEN_TCP_CLIENT_HPP
#define SINEN_TCP_CLIENT_HPP
#include "tcp_socket.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace sinen {
/**
 * @brief TCP client class
 *
 */
class tcp_client {
public:
  /**
   * @brief Construct a new tcp client object
   *
   */
  tcp_client();
  /**
   * @brief Destroy the tcp client object
   *
   */
  ~tcp_client();
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
   * @param maxLength data max length
   * @return true Success
   * @return false Failed
   */
  bool receive(void *data, int maxLength);
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
