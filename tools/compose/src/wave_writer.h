#ifndef NEM_WAVE_WRITER_H
#define NEM_WAVE_WRITER_H
#include "wave.h"
class wave_writer : public wave {
public:
  wave_writer();
  wave_writer(const char *file_path);
  ~wave_writer();

  bool has_file;
  bool has_header;

  bool set_file(const char *file_path);
  void set_header(std::uint32_t wav_length, std::uint16_t ch,
                  std::uint32_t sampling_rate, std::uint16_t bit_depth);

  bool write_file(std::uint8_t *L_data, std::uint8_t *R_data = nullptr);
  bool write_file(std::int16_t *L_data, std::int16_t *R_data = nullptr);

  void close_file();

private:
  FILE *out;
};
#endif