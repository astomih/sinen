#include "wave.h"
wave::wave() { header = {0}; }
wave::~wave() {}

void wave::print_header() {
  std::cout << "[[HEADER]]" << std::endl;
  std::cout << "" << std::endl;

  std::cout << "[RIFF WAVE]" << std::endl;
  std::cout << "Chunk ID          : " << (char)header.chunk_ID[0]
            << (char)header.chunk_ID[1] << (char)header.chunk_ID[2]
            << (char)header.chunk_ID[3] << std::endl;
  std::cout << "Size - 8          : " << header.size_8 << " Byte" << std::endl;
  std::cout << "Wave ID           : " << (char)header.wave_ID[0]
            << (char)header.wave_ID[1] << (char)header.wave_ID[2]
            << (char)header.wave_ID[3] << std::endl;
  std::cout << "" << std::endl;

  std::cout << "[FMT CHUNK]" << std::endl;
  std::cout << "Chunk ID          : " << (char)header.fmt.chunk_ID[0]
            << (char)header.fmt.chunk_ID[1] << (char)header.fmt.chunk_ID[2]
            << (char)header.fmt.chunk_ID[3] << std::endl;
  std::cout << "Chunk Size        : " << header.fmt.chunksize << " Byte"
            << std::endl;
  std::cout << "Format ID         : " << header.fmt.format_ID << std::endl;
  std::cout << "Channels          : " << header.fmt.number_of_channels
            << std::endl;
  std::cout << "Sampling rate     : " << header.fmt.sampling_rate << " Hz"
            << std::endl;
  std::cout << "Byte per second   : " << header.fmt.byte_per_sec << " B/s"
            << std::endl;
  std::cout << "Block size        : " << header.fmt.blocksize << " Byte"
            << std::endl;
  std::cout << "Bit depth         : " << header.fmt.bit_depth << " bit"
            << std::endl;
  std::cout << "" << std::endl;

  std::cout << "[DATA CHUNK]" << std::endl;
  std::cout << "Chunk ID          : " << (char)header.data.chunk_ID[0]
            << (char)header.data.chunk_ID[1] << (char)header.data.chunk_ID[2]
            << (char)header.data.chunk_ID[3] << std::endl;
  std::cout << "Data size         : " << header.data.datasize << " Byte"
            << std::endl;
  std::cout << "" << std::endl;

  return;
}

int32_t wave::length() { return header.data.datasize / header.fmt.blocksize; }

double wave::length_sec() {
  return (double)header.data.datasize / header.fmt.byte_per_sec;
}