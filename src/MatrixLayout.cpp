// MatrixLayout.cpp
#include "MatrixLayout.h"
#include "Globals.h"
#include <FastLED.h> // Needed for qadd8, qsub8, mul8

int XY(uint8_t x, uint8_t y) {
  // Serpentine layout: even columns go down, odd go up
  if (x & 0x01) {
    // odd column, reverse y
    return qadd8(mul8(x, ROWS), qsub8(qsub8(ROWS, 1), y));
  } else {
    // even column, straight y
    return qadd8(mul8(x, ROWS), y);
  }
}
