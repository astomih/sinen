#ifndef SINEN_FILE_HPP
#define SINEN_FILE_HPP

#include <core/buffer/buffer.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>

namespace sinen {
class File {
public:
  /**
   * @brief Construct a new file object
   *
   */
  File();
  /**
   * @brief Destroy the file object
   *
   */
  ~File();
  /**
   * @brief Open a file
   *
   * @param filename File name
   * @param mode Open mode
   * - "r": Open a file for reading. The file must exist.
   * - "w": Create an empty file for writing. If a file with the same name
already
   *        exists its content is erased and the file is treated as a new empty
file.
   * - "a": Append to a file. Writing operations append data at the end of the
file. The file is created if it does not exist.
   * - "r+": Open a file for update both reading and writing. The file must
exist.
   * - "w+": Create an empty file for both reading and writing. If a file with
the same name already exists its content is erased and the file is treated as a
new empty file.
   * - "a+": Open a file for reading and appending.
   * @return true Success to open
   * @return false Failed to open
   */
  bool open(StringView filename, StringView mode);
  void close();
  Buffer read(std::size_t size);
  void write(const Buffer &buffer);
  void seek(const Int64 &offset);
  Int64 tell();
  Int64 size();

private:
  void *stream;
};
} // namespace sinen
#endif // !SINEN_FILE_HPP