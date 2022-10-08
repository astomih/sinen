#include "wave_writer.h"
wave_writer::wave_writer() {
  has_file = false;
  has_header = false;
  out = NULL;
}

wave_writer::wave_writer(const char *file_path) {
  has_file = false;
  has_header = false;
  out = NULL;
  if (set_file(file_path)) {
    has_file = true;
  }
}

wave_writer::~wave_writer() { close_file(); }

bool wave_writer::set_file(const char *file_path) {
  out = fopen64(file_path, "wb");
  if (!out) {
    std::cerr << "Error: File couldn't open " << file_path << std::endl;
    has_file = false;
    return false;
  }
  return true;
}

void wave_writer::set_header(std::uint32_t wav_length, std::uint16_t ch,
                             std::uint32_t sampling_rate,
                             std::uint16_t bit_depth) {
  uint16_t byte_depth = bit_depth / 8;

  memcpy(header.data.chunk_ID, "data", 4);
  header.data.datasize = byte_depth * ch * wav_length;

  // fmt chunk
  memcpy(header.fmt.chunk_ID, "fmt ", 4);
  header.fmt.chunksize = 16;
  header.fmt.format_ID = 1;
  header.fmt.number_of_channels = ch;
  header.fmt.sampling_rate = sampling_rate;
  header.fmt.byte_per_sec = byte_depth * ch * sampling_rate;
  header.fmt.blocksize = byte_depth * ch;
  header.fmt.bit_depth = bit_depth;

  memcpy(header.chunk_ID, "RIFF", 4);
  header.size_8 = header.data.datasize + 44 - 8;
  memcpy(header.wave_ID, "WAVE", 4);
  has_header = true;
}

bool wave_writer::write_file(std::uint8_t *L_data, std::uint8_t *R_data) {
  if (!has_file) {
    std::cerr << "NO OUTPUT FILE" << std::endl;
    return false;
  }

  if (!has_header) {
    std::cerr << "NO DATA" << std::endl;
    return false;
  }

  fseeko64(out, 0, SEEK_SET);

  // header
  fwrite(&header, sizeof(wave_header), 1, out);

  // data
  std::uint32_t length_temp = length();

  if (header.fmt.number_of_channels == 1) {
    fwrite(L_data, 1, length_temp, out);

  } else {
    std::uint8_t *data_temp = new std::uint8_t[length_temp * 2];
    for (uint32_t i = 0; i < length_temp; i++) {
      data_temp[2 * i] = L_data[i];
      data_temp[2 * i + 1] = R_data[i];
    }
    fwrite(data_temp, 1, length_temp * 2, out);
    delete[] data_temp;
  }

  return true;
}

bool wave_writer::write_file(std::int16_t *L_data, std::int16_t *R_data) {
  if (!has_file) {
    std::cerr << "NO OUTPUT FILE" << std::endl;
    return false;
  }

  if (!has_header) {
    std::cerr << "NO DATA" << std::endl;
    return false;
  }

  fseeko64(out, 0, SEEK_SET);

  // header
  fwrite(&header, sizeof(wave_header), 1, out);

  // data
  std::uint32_t length_temp = length();

  if (header.fmt.number_of_channels == 1) {
    fwrite(L_data, 2, length_temp, out);

  } else {
    std::int16_t *data_temp = new std::int16_t[length_temp * 2];
    for (std::uint32_t i = 0; i < length_temp; i++) {
      data_temp[2 * i] = L_data[i];
      data_temp[2 * i + 1] = R_data[i];
    }
    fwrite(data_temp, 2, length_temp * 2, out);
    delete[] data_temp;
  }

  return true;
}

void wave_writer::close_file() {
  fclose(out);
  has_file = false;
}