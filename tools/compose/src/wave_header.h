#ifndef NEM_WAVE_HEADER_H
#define NEM_WAVE_HEADER_H
#include "data_chunk.h"
#include "fmt_chunk.h"
class wave_header {
public:
  std::uint8_t chunk_ID[4];
  std::uint32_t size_8;
  std::uint8_t wave_ID[4];
  fmt_chunk fmt;
  data_chunk data;
};
#endif