// WaveRenderer.cpp
#include "WaveRenderer.h"

static std::vector<Wave> waves;
static unsigned long next_wave_time = 0;

// These mod values and palette should be defined externally
extern float mod1;
extern float mod2;
extern CRGBPalette16 currentPalette;

void initWaves() {
  waves.clear();
  next_wave_time = 0;
}

void updateWaves() {
  unsigned long now = millis();

  if (now > next_wave_time && waves.size() < 10) {
    Wave w;
    w.startTime = now;

    uint8_t index = random8();
    uint8_t modulated_index = index + (int8_t)(mod2 * 64);
    w.color = ColorFromPalette(currentPalette, modulated_index);

    w.feather_width = map(mod1 * 100, -100, 100, 3, 10);
    w.duration = map(mod2 * 100, -100, 100, 1000, 3000);

    waves.push_back(w);
    next_wave_time = now + random(100, 500);
  }
}

void renderWaves(CRGB* matrix, int matrixSize, CRGB* bar_1, CRGB* bar_2, int barSize, uint8_t cols, uint8_t rows, int (*XY)(uint8_t, uint8_t)) {
  fadeToBlackBy(matrix, matrixSize, 20);
  fadeToBlackBy(bar_1, barSize, 20);
  fadeToBlackBy(bar_2, barSize, 20);

  unsigned long now = millis();

  for (size_t w = 0; w < waves.size(); ) {
    unsigned long age = now - waves[w].startTime;
    if (age >= waves[w].duration) {
      waves.erase(waves.begin() + w);
      continue;
    }

    unsigned long matrix_phase = waves[w].duration * 0.25;
    unsigned long bar_phase = waves[w].duration - matrix_phase;

    CRGB base_color = waves[w].color;
    float feather = waves[w].feather_width;

    if (age < matrix_phase) {
      float progress = age / (float)matrix_phase;
      float radius = (cols / 2.0f) * progress;
      int center = cols / 2;

      for (uint8_t x = 0; x < cols; x++) {
        float distance = abs((int)x - center);
        float brightness = max(0.0f, 1.0f - abs(distance - radius) / feather);
        CRGB color = base_color;
        color.nscale8(brightness * 255);

        for (uint8_t y = 0; y < rows; y++) {
          int idx = XY(x, y);
          if (idx >= 0 && idx < matrixSize) matrix[idx] += color;
        }
      }
    } else {
      float progress = (age - matrix_phase) / (float)bar_phase;
      float radius = barSize * progress;

      for (int i = 0; i < barSize; i++) {
        float distance = i;
        float brightness = max(0.0f, 1.0f - abs(distance - radius) / feather);
        CRGB color = base_color;
        color.nscale8(brightness * 255);

        bar_1[i] += color;
        bar_2[i] += color;
      }
    }
    ++w;
  }
}
