// StrobeEffect.cpp
#include "StrobeEffect.h"
#include "Globals.h"
#include "LEDHelpers.h"
#include <FastLED.h>

extern CRGBPalette16 currentPalette;

namespace {
  unsigned long strobeStartTime = 0;
  unsigned long strobeDuration = 0;
  bool strobeCurrentlyActive = false;
  CRGB strobeColor;
  int strobeFrequency = 20; // Hz
}

void startStrobe(unsigned long durationMs, bool useWhite) {
  strobeStartTime = millis();
  strobeDuration = durationMs;
  strobeCurrentlyActive = true;
  if (useWhite) {
    strobeColor = CRGB::White;
  } else {
    strobeColor = ColorFromPalette(currentPalette, random8());
  }
}

void stopStrobe() {
  strobeCurrentlyActive = false;
}

void updateStrobeEffect() {
  if (!strobeCurrentlyActive) return;

  unsigned long now = millis();
  if (now - strobeStartTime > strobeDuration) {
    strobeCurrentlyActive = false;
    return;
  }

  // Compute current frame within the strobe cycle
  unsigned long cycleMs = 1000 / strobeFrequency;
  bool onFrame = ((now / cycleMs) % 2) == 0;

  if (onFrame) {
    fill_solid(matrix, NUM_LEDS_MATRIX, strobeColor);
    fill_solid(bar_1, NUM_LEDS_BAR, strobeColor);
    fill_solid(bar_2, NUM_LEDS_BAR, strobeColor);
  } else {
    fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);
    fill_solid(bar_1, NUM_LEDS_BAR, CRGB::Black);
    fill_solid(bar_2, NUM_LEDS_BAR, CRGB::Black);
  }
}

bool isStrobeActive() {
  return strobeCurrentlyActive;
}
