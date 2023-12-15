#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <cstdint>

struct RecordingInfo{
  int baud;
  bool inverted;
};

struct RecordingFrame {
  uint16_t delta;
  uint8_t data;
  uint8_t cheksum;
};

#endif