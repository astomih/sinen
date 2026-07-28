#include <core/license_report.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {
class OutputFile {
public:
  explicit OutputFile(std::filesystem::path path) : path(std::move(path)) {
    std::error_code error;
    std::filesystem::remove(this->path, error);
  }

  ~OutputFile() {
    std::error_code error;
    std::filesystem::remove(path, error);
  }

  std::filesystem::path path;
};
} // namespace

TEST(LicenseReportTests, WritesSelfContainedEscapedHtml) {
  OutputFile output(std::filesystem::current_path() /
                    "sinen_license_report_test.html");

  ASSERT_TRUE(sinen::LicenseReport::writeHtml(output.path.string()));

  std::ifstream input(output.path, std::ios::binary);
  ASSERT_TRUE(input.good());
  const std::string html((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());

  EXPECT_NE(html.find("<!doctype html>"), std::string::npos);
  EXPECT_NE(html.find("LuaBridge3"), std::string::npos);
  EXPECT_NE(html.find("Luau Debugger"), std::string::npos);
  EXPECT_NE(html.find("&lt;COPYRIGHT HOLDER&gt;"), std::string::npos);
  EXPECT_EQ(html.find("<COPYRIGHT HOLDER>"), std::string::npos);
}
