// system include
#include <boost/spirit/home/qi/numeric/bool.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <ios>
#include <iostream>
#include <math.h>
#include <memory>
#include <stdio.h>
#include <string>
#include <unistd.h> //for sleep

// third party
#include "wave_writer.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <boost/spirit/include/qi.hpp>

int ch = 1;
int sampling_rate = 8000;
int bit_depth = 8;
int play_time_sec = 5;
int freq = 500;

int length = sampling_rate * play_time_sec;

const double PI_2 = 6.2831853;

int main(int argc, char **argv) {
  ALCdevice *device;
  ALCcontext *context;
  uint8_t data[length];
  ALuint buffer, source;

  // open device.
  device = alcOpenDevice(NULL);
  context = alcCreateContext(device, NULL);
  alcMakeContextCurrent(context);
  // generate buffer
  alGenBuffers(1, &buffer);

  std::string file_path;
  std::cout << "> ";
  std::cin >> file_path;
  if (!file_path.ends_with(".nem")) {
    std::cerr << "This is invalid extension." << std::endl;
    return -1;
  }
  std::ifstream file;
  std::string file_data;
  try {
    file.open(file_path, std::ios_base::in);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  std::string buf;
  while (std::getline(file, buf)) {
    file_data += buf + "\n";
  }
  file.close();
  std::string::iterator first = file_data.begin(), last = file_data.end();
  namespace qi = boost::spirit::qi;
  bool success = qi::parse(first, last, qi::bool_);
  if (success) {
    std::cout << "OK" << std::endl;
  } // OK が出力される
  std::cout << file_data << std::endl;

  // generate signal
  for (int i = 0; i < length; ++i) {
    data[i] =
        (uint8_t)((sin(PI_2 * i * freq / sampling_rate) + 1.0) / 2.0 * 255.0);
  }
  // input signal to buffer
  alBufferData(buffer, AL_FORMAT_MONO8, data, sizeof(data), sampling_rate);
  // generate source
  alGenSources(1, &source);
  // attach source to buffer
  alSourcei(source, AL_BUFFER, buffer);
  // play
  alSourcePlay(source);
  sleep(4);

  // delete buffer,source,etc...
  alSourceStop(source);
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);

  if (argc < 2)
    return 0;
  // load file
  auto wave_out = std::make_unique<wave_writer>(argv[1]);
  if (!wave_out->has_file)
    return 1;

  // build header
  wave_out->set_header(length, ch, sampling_rate, bit_depth);
  wave_out->print_header();
  // write wav
  wave_out->write_file(data);
  return 0;
}