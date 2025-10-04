#include "StrobeEffect.h"
#include "Globals.h"
#include "LEDHelpers.h"
#include <FastLED.h>

using namespace LEDStore;

extern CRGBPalette16 currentPalette;

namespace {
  unsigned long strobeStartTime = 0;
  unsigned long strobeDuration  = 0;
  bool          strobeActiveNow = false;
  CRGB          strobeColor;
  int           strobeFrequency = 20; // Hz
}

static inline void startBurst(unsigned long durationMs, bool useWhite) {
  strobeStartTime  = millis();
  strobeDuration   = durationMs;
  strobeActiveNow  = true;
  strobeColor      = useWhite ? CRGB::White : ColorFromPalette(currentPalette, random8());
}

void triggerStrobeBurst(unsigned long durationMs, bool useWhite) {
  // Ignore if a burst is already running; drop extra presses
  if (!strobeActiveNow) startBurst(durationMs, useWhite);
}

void stopStrobe() { strobeActiveNow = false; }

bool isStrobeActive() { return strobeActiveNow; }

void updateStrobeEffect() {
  const unsigned long now = millis();
  if (!strobeActiveNow) return;

  if (now - strobeStartTime >= strobeDuration) {
    strobeActiveNow = false;
    return;
  }

  const unsigned long cycleMs = 1000UL / strobeFrequency;
  const bool onFrame = (((now - strobeStartTime) / cycleMs) % 2U) == 0U;

  fill_solid(matrix, NUM_LEDS_MATRIX, onFrame ? strobeColor : CRGB::Black);
  fill_solid(bar_1,  NUM_LEDS_BAR,   onFrame ? strobeColor : CRGB::Black);
  fill_solid(bar_2,  NUM_LEDS_BAR,   onFrame ? strobeColor : CRGB::Black);
  fill_solid(bar_3,  NUM_LEDS_BAR,   onFrame ? strobeColor : CRGB::Black);
  fill_solid(bar_4,  NUM_LEDS_BAR,   onFrame ? strobeColor : CRGB::Black);
}