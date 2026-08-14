#include <platform/io/external_file.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
using namespace std::string_literals;

class TemporaryFile {
public:
  TemporaryFile() {
    path = std::filesystem::current_path() / "sinen_external_file_test.bin";
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
  }

  ~TemporaryFile() {
    std::error_code error;
    std::filesystem::remove(path, error);
  }

  std::filesystem::path path;
  const std::string contents = "external-file-data\0with-binary"s;
};
} // namespace

TEST(ExternalFileTests, ExposesOnlyFileMetadataAndReadsSelectedFile) {
  TemporaryFile temporary;
  sinen::ExternalFile file(sinen::String(temporary.path.string()));

  EXPECT_EQ(file.name(), "sinen_external_file_test.bin");
  EXPECT_EQ(file.extension(), ".bin");

  auto buffer = file.read();
  ASSERT_TRUE(buffer.has_value());
  ASSERT_EQ(buffer->size(), temporary.contents.size());
  EXPECT_EQ(std::memcmp(buffer->data(), temporary.contents.data(),
                        temporary.contents.size()),
            0);
}
