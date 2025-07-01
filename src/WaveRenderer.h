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
void spawnNewWave();
void renderWaves();
