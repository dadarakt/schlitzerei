// SearchlightPattern.cpp
#include "SearchlightPattern.h"
#include "Globals.h"
#include "LEDHelpers.h"

using namespace LEDStore;

extern double mod1;
extern double mod2;

bool searchlight_active = false;
unsigned long searchlight_start = 0;
unsigned long searchlight_duration = 1500;

void updateSearchlight() {
  static unsigned long next_activation_time = 0;
  static unsigned long active_duration = 0;

  unsigned long now = millis();

  if (searchlight_active) {
    if (now - searchlight_start > active_duration) {
      searchlight_active = false;
      next_activation_time = now + random(10000, 20000); // 10–20 sec off
    }
  } else {
    if (now >= next_activation_time) {
      searchlight_active = true;
      searchlight_start = now;
      active_duration = random(10000, 20000); // 10–20 sec on
    }
  }
}

void renderRedGlimmerNew(CRGB* leds, int num_leds, uint16_t offset = 0) {
  // sine wave from 0-1
  int t = (millis() / 512) % 65535;
  int intensity = map(sin8(t), -1, 1, 0, 255);

  for (int i = 0; i < num_leds; i++) {
    leds[i] = CRGB(intensity, 0, 0);
  }
}

void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset = 0) {
  float speed = 25 + ((mod1 + 1.0f) / 2.0f) * 4.0f; // modulate pulsing speed
  float pulse = ((sin8(millis() / speed) - 128) / 128.0f + 1.0f) * 1.1f; // heartbeat-like

  for (int i = 0; i < num_leds; i++) {
    uint8_t noise = inoise8(i * 10, millis() / 8 + offset);
    int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) / 2.0f) * 80.0f;
    uint8_t red = min(red_intensity, 180);
    leds[i] = CRGB(red, red / 10, red / 20); // warm red, avoid whiteout
  }
}

void renderRedGlimmerMatrix() {
  float speed = 25 + ((mod1 + 1.0f) / 2.0f) * 8.0f; // modulate pulsing speed
  float pulse = (sin8(millis() / speed) - 128) / 128.0f + 1.0f;
  for (uint8_t x = 0; x < COLS; x++) {
    for (uint8_t y = 0; y < ROWS; y++) {
      uint8_t noise = inoise8(x * 15, y * 15, millis() / 10);
      int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) / 2.0f) * 80.0f;
      uint8_t red = min(red_intensity, 180);
      matrix[XY(x, y)] = CRGB(red, red / 10, red / 20);
    }
  }
}

// Helper to apply the moving "searchlight" brightness to a linear array
static inline void applySweepToStrip(CRGB* leds, int count, float base, float pos, float width, bool reverse) {
  for (int i = 0; i < count; ++i) {
    float localIndex = reverse ? (count - 1 - i) : i;
    float global_pos = base + localIndex;
    float brightness = max(0.0f, 1.0f - fabsf(global_pos - pos) / width);
    uint8_t v = (uint8_t)(brightness * 255.0f + 0.5f);
    CRGB white(v, v, v);
    leds[i] += white; // additive “white” sweep
  }
}

// Helper to apply the moving sweep across matrix columns
static inline void applySweepToMatrix(float base, float pos, float width) {
  for (uint8_t x = 0; x < COLS; ++x) {
    float global_pos = base + x; // left-to-right
    float brightness = max(0.0f, 1.0f - fabsf(global_pos - pos) / width);
    uint8_t v = (uint8_t)(brightness * 255.0f + 0.5f);
    CRGB white(v, v, v);
    for (uint8_t y = 0; y < ROWS; ++y) {
      matrix[XY(x, y)] += white;
    }
  }
}

void renderSearchlightCombined() {
  if (!searchlight_active) return;

  // Build a simple 1D layout across everything:
  // strip_1 → bar_1 → bar_2 → matrix → bar_3 → bar_4 → strip_2
  float base = 0.0f;
  const float off_strip1 = base;                  base += NUM_LEDS_STRIP;
  const float off_bar1   = base;                  base += NUM_LEDS_BAR;
  const float off_bar2   = base;                  base += NUM_LEDS_BAR;
  const float off_matrix = base;                  base += COLS;
  const float off_bar3   = base;                  base += NUM_LEDS_BAR;
  const float off_bar4   = base;                  base += NUM_LEDS_BAR;
  const float off_strip2 = base;                  base += NUM_LEDS_STRIP;
  const float total_width = base;

  // Sweep position & width
  float oscillation = sinf(millis() / 1000.0f) * 0.5f + 0.5f; // 0..1
  float pos = oscillation * total_width;
  float width = 6.0f + ((mod2 + 1.0f) / 2.0f) * 10.0f; // modulated, softer edges

  // Left side reversed (inward), center/right forward:
  applySweepToStrip(strip_1, NUM_LEDS_STRIP, off_strip1, pos, width, /*reverse=*/true);
  applySweepToStrip(bar_1,   NUM_LEDS_BAR,   off_bar1,   pos, width, /*reverse=*/true);
  applySweepToStrip(bar_2,   NUM_LEDS_BAR,   off_bar2,   pos, width, /*reverse=*/true);

  applySweepToMatrix(off_matrix, pos, width);

  applySweepToStrip(bar_3,   NUM_LEDS_BAR,   off_bar3,   pos, width, /*reverse=*/false);
  applySweepToStrip(bar_4,   NUM_LEDS_BAR,   off_bar4,   pos, width, /*reverse=*/false);
  applySweepToStrip(strip_2, NUM_LEDS_STRIP, off_strip2, pos, width, /*reverse=*/false);
}

void renderSearchlightPattern() {
  updateSearchlight();

  // Base “glimmer” layers for all panels
  renderRedGlimmer(bar_1, NUM_LEDS_BAR);
  renderRedGlimmer(bar_2, NUM_LEDS_BAR, 1000);
  renderRedGlimmer(bar_3, NUM_LEDS_BAR, 2000);
  renderRedGlimmer(bar_4, NUM_LEDS_BAR, 3000);

  renderRedGlimmer(strip_1, NUM_LEDS_STRIP, 1000);
  renderRedGlimmer(strip_2, NUM_LEDS_STRIP);

  renderRedGlimmerMatrix();

  // Add the moving “searchlight” sweep on top
  renderSearchlightCombined();
}
