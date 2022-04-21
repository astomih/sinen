#pragma once
#include <memory>

namespace nen {
class tcp_socket {
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  tcp_socket();
  ~tcp_socket();
};
} // namespace nen