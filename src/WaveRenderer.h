// WaveRenderer.h
#pragma once
#include <FastLED.h>
#include <vector>

struct Wave {
  unsigned long startTime;
  unsigned long duration;
  CRGB color;
  float feather_width;
};

void initWaves();
void updateWaves();
void renderWaves(CRGB* matrix, int matrixSize, CRGB* bar_1, CRGB* bar_2, int barSize, uint8_t cols, uint8_t rows, int (*XY)(uint8_t, uint8_t));
