#include <sinen/logger/logger.hpp>

namespace sinen {
class imgui_logger : public sinen::logger::interface {
public:
  virtual void debug(std::string_view) final override;
  virtual void info(std::string_view) final override;
  virtual void error(std::string_view) final override;
  virtual void warn(std::string_view) final override;
  virtual void fatal(std::string_view) final override;
};
} // namespace sinen