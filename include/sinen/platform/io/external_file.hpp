#ifndef SINEN_EXTERNAL_FILE_HPP
#define SINEN_EXTERNAL_FILE_HPP

#include <core/buffer/buffer.hpp>
#include <core/data/string.hpp>

#include <optional>

namespace sinen {

/**
 * A read-only file capability created by an explicit user action, such as a
 * file dialog selection or a drag-and-drop operation.
 *
 * The native path is intentionally not exposed to Luau. Runtime asset paths
 * remain sandboxed and cannot be upgraded into ExternalFile values.
 */
class ExternalFile {
public:
  explicit ExternalFile(String path);

  static constexpr const char *metaTableName() { return "sn.ExternalFile"; }

  std::optional<Buffer> read() const;
  String name() const;
  String extension() const;

private:
  String nativePath;
};

} // namespace sinen

#endif // SINEN_EXTERNAL_FILE_HPP
