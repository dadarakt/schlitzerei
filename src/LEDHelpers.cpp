#include "LEDHelpers.h"
#include "Globals.h"

void fill_solid_all(CRGB color) {
  fill_solid(bar_1, NUM_LEDS_BAR, color);
  fill_solid(bar_2, NUM_LEDS_BAR, color);
  fill_solid(strip_1, NUM_LEDS_STRIP, color);
  fill_solid(strip_2, NUM_LEDS_STRIP, color);
  fill_solid(matrix, NUM_LEDS_MATRIX, color);
}

void add_solid(CRGB* leds, int num_leds, CRGB color) {
  for (int i = 0; i < num_leds; i++) {
    leds[i] += color;
  }
}

void add_solid_all(CRGB color) {
  add_solid(bar_1, NUM_LEDS_BAR, color);
  add_solid(bar_2, NUM_LEDS_BAR, color);
  add_solid(strip_1, NUM_LEDS_STRIP, color);
  add_solid(strip_2, NUM_LEDS_STRIP, color);
  add_solid(matrix, NUM_LEDS_MATRIX, color);
}


void fade_leds(CRGB* leds, int num_leds, int fade_amount) {
  for (int i = 0; i < num_leds; i++) {
    leds[i].fadeToBlackBy(fade_amount);
  }
}

void fade_all(int fade_amount) {
  fade_leds(bar_1, NUM_LEDS_BAR, fade_amount);
  fade_leds(bar_2, NUM_LEDS_BAR, fade_amount);
  fade_leds(matrix, NUM_LEDS_MATRIX, fade_amount);
  fade_leds(strip_1, NUM_LEDS_STRIP, fade_amount);
  fade_leds(strip_2, NUM_LEDS_STRIP, fade_amount);
}

