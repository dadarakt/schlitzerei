#include "StrobeEffect.h"
#include "Globals.h"
#include "LEDHelpers.h"
#include <FastLED.h>

using namespace LEDStore;

extern CRGBPalette16 currentPalette;

namespace {
  unsigned long strobeStartTime = 0;
  bool          strobeActiveNow = false;
  CRGB          strobeColor;
  int           strobeFrequency = 15; // Hz
}

void startStrobeContinuous(bool useWhite) {
  if (strobeActiveNow) return;           // already running
  strobeStartTime  = millis();
  strobeActiveNow  = true;
  strobeColor      = useWhite ? CRGB::White : ColorFromPalette(currentPalette, random8());
}

void stopStrobe() { strobeActiveNow = false; }

bool isStrobeActive() { return strobeActiveNow; }

void updateStrobeEffect() {
  if (!strobeActiveNow) return;

  const unsigned long now = millis();
  const unsigned long cycleMs = 1000UL / strobeFrequency;
  const bool onFrame = (((now - strobeStartTime) / cycleMs) % 2U) == 0U;

  if (onFrame) {
    fill_solid(matrix, NUM_LEDS_MATRIX, strobeColor);
    // fill_solid(bar_1,  NUM_LEDS_BAR,    strobeColor);
    // fill_solid(bar_2,  NUM_LEDS_BAR,    strobeColor);
    fill_solid(bar_3,  NUM_LEDS_BAR,    strobeColor);
    fill_solid(bar_4,  NUM_LEDS_BAR,    strobeColor);
  } else {
    fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);
    // fill_solid(bar_1,  NUM_LEDS_BAR,    CRGB::Black);
    // fill_solid(bar_2,  NUM_LEDS_BAR,    CRGB::Black);
    fill_solid(bar_3,  NUM_LEDS_BAR,    CRGB::Black);
    fill_solid(bar_4,  NUM_LEDS_BAR,    CRGB::Black);
  }
}
