#ifndef NEM_FMT_CHUNK_H
#define NEM_FMT_CHUNK_H
#include <cstdint>
class fmt_chunk {
public:
  std::uint8_t chunk_ID[4];
  std::uint32_t chunksize;
  std::uint16_t format_ID;
  std::uint16_t number_of_channels;
  std::uint32_t sampling_rate;
  std::uint32_t byte_per_sec;
  std::uint16_t blocksize;
  std::uint16_t bit_depth;
};
#endif