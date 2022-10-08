#ifndef NEM_DATA_CHUNK_H
#define NEM_DATA_CHUNK_H
#include <cstdint>
class data_chunk {
public:
  std::uint8_t chunk_ID[4];
  std::uint32_t datasize;
};
#endif