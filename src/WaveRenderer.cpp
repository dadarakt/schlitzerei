// WaveRenderer.cpp
#include "WaveRenderer.h"
#include "Globals.h"
#include "MatrixLayout.h"

static std::vector<Wave> waves;
static unsigned long next_wave_time = 0;
const unsigned long wave_lifetime = 1500;
const uint8_t max_waves = 50;  // bump up for more layering

void initWaves() {
  waves.clear();
  next_wave_time = 0;
}

void spawnNewWave() {
  Wave w;
  w.startTime = millis();

  uint8_t index = random8();
  w.color = ColorFromPalette(currentPalette, index);

  w.feather_width = random(3, 8);  // width

  // Random speed: total wave lifetime between 1000–2500ms
  w.duration = random(1000, 2500);

  waves.push_back(w);
  next_wave_time = millis() + random(100, 500);
}

void renderWaves() {
  fadeToBlackBy(matrix, NUM_LEDS_MATRIX, 5);  // adjust fade rate as needed
  fadeToBlackBy(bar_1, NUM_LEDS_BAR, 5);
  fadeToBlackBy(bar_2, NUM_LEDS_BAR, 5);

  unsigned long now = millis();

  // Maybe spawn new wave
  if (millis() > next_wave_time && waves.size() < max_waves) {
    spawnNewWave();
  }

  for (int w = 0; w < waves.size(); ) {
    unsigned long age = now - waves[w].startTime;

    if (age >= waves[w].duration) {
      waves.erase(waves.begin() + w);
      continue;
    }

    unsigned long matrix_phase = waves[w].duration * 0.25;  // matrix = 25% of duration
    unsigned long bar_phase = waves[w].duration - matrix_phase;

    CRGB base_color = waves[w].color;
    float feather = waves[w].feather_width;

    if (age < matrix_phase) {
      float progress = age / (float)matrix_phase;
      float radius = (COLS / 2.0) * progress;
      int center = COLS / 2;

      for (int x = 0; x < COLS; x++) {
        float distance = abs(x - center);
        float brightness = max(0.0f, 1.0f - abs(distance - radius) / feather);
        CRGB color = base_color;
        color.nscale8(brightness * 255);

        for (int y = 0; y < ROWS; y++) {
          matrix[XY(x, y)] += color;
        }
      }
    } else {
      float progress = (age - matrix_phase) / (float)bar_phase;
      float radius = NUM_LEDS_BAR * progress;

      for (int i = 0; i < NUM_LEDS_BAR; i++) {
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