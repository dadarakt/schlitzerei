#include "CrowdBlinder.h"
#include "Globals.h"
#include <FastLED.h>

using namespace LEDStore;

namespace {
  bool     g_target = false;    // desired state from button
  uint8_t  g_level  = 0;        // 0..255 overlay intensity
  uint16_t g_attack = 200;      // ms fade-in
  uint16_t g_release= 400;      // ms fade-out
  unsigned long g_last = 0;

  inline void addWhiteOverlay(CRGB* leds, int count, uint8_t level) {
    for (int i = 0; i < count; ++i) {
      leds[i].r = qadd8(leds[i].r, level);
      leds[i].g = qadd8(leds[i].g, level);
      leds[i].b = qadd8(leds[i].b, level);
    }
  }
}

void crowdBlinderSetAttackRelease(uint16_t attackMs, uint16_t releaseMs) {
  g_attack  = attackMs ? attackMs : 1;
  g_release = releaseMs ? releaseMs : 1;
}

void crowdBlinderEnable(bool enable) {
  g_target = enable;
}

uint8_t crowdBlinderLevel() {
  return g_level;
}

void crowdBlinderUpdate() {
  const unsigned long now = millis();
  const unsigned long dt  = (g_last == 0) ? 0 : (now - g_last);
  g_last = now;

  // Fade envelope
  if (g_target && g_level < 255) {
    const uint32_t inc = (uint32_t)255 * dt / g_attack;
    g_level = qadd8(g_level, (uint8_t)min<uint32_t>(inc, 255));
  } else if (!g_target && g_level > 0) {
    const uint32_t dec = (uint32_t)255 * dt / g_release;
    g_level = qsub8(g_level, (uint8_t)min<uint32_t>(dec, 255));
  }

  if (g_level == 0) return; // nothing to draw

  addWhiteOverlay(matrix, NUM_LEDS_MATRIX, g_level);
  addWhiteOverlay(bar_1,  NUM_LEDS_BAR,    g_level);
  addWhiteOverlay(bar_2,  NUM_LEDS_BAR,    g_level);
  addWhiteOverlay(bar_3,  NUM_LEDS_BAR,    g_level);
  addWhiteOverlay(bar_4,  NUM_LEDS_BAR,    g_level);
  addWhiteOverlay(strip_1, NUM_LEDS_STRIP, g_level);
  addWhiteOverlay(strip_2, NUM_LEDS_STRIP, g_level);
}
