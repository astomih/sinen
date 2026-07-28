#ifndef SINEN_LICENSE_REPORT_HPP
#define SINEN_LICENSE_REPORT_HPP

#include <core/data/string.hpp>

namespace sinen {
class LicenseReport {
public:
  static bool writeHtml(StringView outputPath);
};
} // namespace sinen

#endif // SINEN_LICENSE_REPORT_HPP
