#ifndef NEM_WAVE_H
#define NEM_WAVE_H
#include "data_chunk.h"
#include "wave_header.h"
#include <iostream>

class wave {
public:
  wave();
  virtual ~wave();

  /**
   * @brief header info print
   *
   */
  void print_header();
  /**
   * @brief data length (byte)
   *
   * @return int32_t
   */
  int32_t length();
  /**
   * @brief data length (seconds)
   *
   * @return double
   */
  double length_sec();

protected:
  wave_header header;
};
#endif