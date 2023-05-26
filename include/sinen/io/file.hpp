#ifndef SINEN_FILE_HPP
#define SINEN_FILE_HPP
#include <list>
#include <memory>
#include <string_view>

namespace sinen {
class file {
public:
  /**
   * @brief Construct a new file object
   *
   */
  file();
  /**
   * @brief Destroy the file object
   *
   */
  ~file();
  enum class mode {
    /**
     * @brief Open a file for reading. The file must exist.
     *
     */
    r,
    /**
     * @brief Create an empty file for writing. If a file with the same name
     * already exists its content is erased and the file is treated as a new
     * empty file.
     *
     */
    w,
    /**
     * @brief Append to a file. Writing operations append data at the end of the
     * file. The file is created if it does not exist.
     */
    a,
    /**
     * @brief Open a file for update both reading and writing. The file must
     * exist.
     *
     */
    rp,
    /**
     * @brief Create an empty file for both reading and writing. If a file with
     * the same name already exists its content is erased and the file is
     * treated as a new empty file.
     *
     */
    wp,
    /**
     * @brief Open a file for reading and appending. All writing operations are
     * performed at the end of the file, protecting the previous content to be
     * overwritten. You can reposition (fseek, rewind) the internal pointer to
     * anywhere in the file for reading, but writing operations will move it
     * back to the end of file. The file is created if it does not exist.
     *
     */
    ap,
    /**
     * @brief Open a file for reading. The file must exist.
     *
     */
    rb,
    /**
     * @brief Create an empty file for writing. If a file with the same name
     * already exists its content is erased and the file is treated as a new
     * empty file.
     *
     */
    wb,
    /**
     * @brief Append to a file. Writing operations append data at the end of the
     * file. The file is created if it does not exist.
     */
    ab,
    /**
     * @brief Open a file for update both reading and writing. The file must
     * exist.
     *
     */
    rpb,
    /**
     * @brief Create an empty file for both reading and writing. If a file with
     * the same name already exists its content is erased and the file is
     * treated as a new empty file.
     *
     */
    wpb,
    /**
     * @brief Open a file for reading and appending. All writing operations are
     * performed at the end of the file, protecting the previous content to be
     * overwritten. You can reposition (fseek, rewind) the internal pointer to
     * anywhere in the file for reading, but writing operations will move it
     * back to the end of file. The file is created if it does not exist.
     *
     */
    apb
  };
  /**
   * @brief Open a file
   *
   * @param filename File name
   * @param mode Open mode
   * @return true Success to open
   * @return false Failed to open
   */
  bool open(const char *filename, const mode &mode);
  /**
   * @brief Open a file
   *
   * @param filename File name
   * @param mode Open mode
   * @return true Success to open
   * @return false Failed to open
   */
  bool open(std::string_view filename, const mode &mode);
  /**
   * @brief Open a file
   *
   * @param filename File name
   * @param mode Open mode
   * @return true Success to open
   * @return false Failed to open
   */
  bool open(const char *filename, const char *mode);
  /**
   * @brief Open a file
   *
   * @param filename File name
   * @param mode Open mode
   * @return true Success to open
   * @return false Failed to open
   */
  bool open(std::string_view filename, const char *mode);
  /**
   * @brief Close a file
   *
   */
  void close();
  /**
   * @brief Get the file size
   *
   * @param buffer Dst buffer
   * @param size Read size
   * @param maxnum Max read size
   */
  void read(void *buffer, std::size_t size, std::size_t maxnum);
  /**
   * @brief Write data to a file
   *
   * @param buffer Src buffer
   * @param size Write size
   * @param num Write num
   */
  void write(const void *buffer, std::size_t size, std::size_t num);
  /**
   * @brief Seek to a position in a file
   *
   * @param offset Offset
   * @param whence Whence
   */
  void seek(const int64_t &offset, int whence);
  /**
   * @brief Get the current position of the file pointer
   *
   * @return int64_t Current position
   */
  int64_t tell();
  /**
   * @brief Get the file size
   *
   * @return std::int64_t File size
   */
  int64_t size();
  /**
   * @brief Open mode to string
   *
   * @param mode mode
   * @return std::string string
   */
  std::string open_mode_to_string(const mode &mode);

private:
  class impl;
  std::unique_ptr<impl> m_impl;
};
} // namespace sinen
#endif // !SINEN_FILE_HPP