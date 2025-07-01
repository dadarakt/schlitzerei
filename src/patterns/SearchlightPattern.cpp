// SearchlightPattern.cpp
#include "SearchlightPattern.h"
#include "Globals.h"
#include "LEDHelpers.h"

extern double mod1;
extern double mod2;

bool searchlight_active = false;
unsigned long searchlight_start = 0;
unsigned long searchlight_duration = 1500;

void initSearchlight() {
  searchlight_active = false;
  searchlight_start = 0;
}

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

void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset = 0) {
  float speed = 20 + mod1 * 7.0f; // modulate pulsing speed
  float pulse = ((sin8(millis() / speed) - 128) / 128.0f + 1.0f) * 1.2f; // stronger pulsing // heartbeat-like
  for (int i = 0; i < num_leds; i++) {
    uint8_t noise = inoise8(i * 10, millis() / 8 + offset);
    uint8_t red = qadd8(noise, 64) * pulse;
    leds[i] = CRGB(red, 0, 0);
  }
}

void renderRedGlimmerMatrix() {
  float speed = 20 + mod1 * 15.0f; // modulate pulsing speed
  float pulse = (sin8(millis() / speed) - 128) / 128.0f + 1.0f;
  for (uint8_t x = 0; x < COLS; x++) {
    for (uint8_t y = 0; y < ROWS; y++) {
      uint8_t noise = inoise8(x * 15, y * 15, millis() / 10);
      uint8_t red = qadd8(noise, 64) * pulse;
      matrix[XY(x, y)] = CRGB(red, 0, 0);
    }
  }
}

void renderSearchlightCombined() {
  if (!searchlight_active) return;

  float total_width = NUM_LEDS_BAR + COLS + NUM_LEDS_BAR;
  float oscillation = sinf(millis() / 1000.0f) * 0.5f + 0.5f; // oscillate 0.0 to 1.0
  float pos = oscillation * total_width;
  float width = 6 + mod2 * 10.0f; // softer edges modulated

  // Bar 1
  for (int i = 0; i < NUM_LEDS_BAR; i++) {
    float global_pos = NUM_LEDS_BAR - 1 - i;
    float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
    CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
    bar_1[i] += white;
  }

  // Matrix
  for (uint8_t x = 0; x < COLS; x++) {
    float global_pos = NUM_LEDS_BAR + x;
    float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
    CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
    for (uint8_t y = 0; y < ROWS; y++) {
      matrix[XY(x, y)] += white;
    }
  }

  // Bar 2
  for (int i = 0; i < NUM_LEDS_BAR; i++) {
    float global_pos = NUM_LEDS_BAR + COLS + i;
    float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
    CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
    bar_2[i] += white;
  }
}

void renderSearchlightPattern() {
  static bool strobe_active = false;
  static unsigned long strobe_start = 0;
  static unsigned long next_strobe_time = 0;
  static unsigned long strobe_duration = 0;
  static unsigned long last_flash = 0;
  renderRedGlimmer(bar_1, NUM_LEDS_BAR);
  renderRedGlimmer(bar_2, NUM_LEDS_BAR, 1000);
  // Strobe logic
  unsigned long now = millis();
  if (!strobe_active && now > next_strobe_time) {
    strobe_active = true;
    strobe_start = now;
    strobe_duration = random(1000, 3000); // 1–3 seconds
    last_flash = 0;
  }

  if (strobe_active) {
    if (now - last_flash > 100) { // ~20Hz
      fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::White);
      last_flash = now;
    }
    if (now - strobe_start > strobe_duration) {
      strobe_active = false;
      next_strobe_time = now + random(5000, 15000); // next strobe in 5–15s
    }
  } else {
    renderRedGlimmerMatrix();
  }
  renderSearchlightCombined();
}
