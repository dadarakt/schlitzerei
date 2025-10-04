#include "PanelPulse.h"
#include "Globals.h"
#include "ColorPalettes.h"

using namespace LEDStore;

extern CRGBPalette16 currentPalette;

// external modulators (provided elsewhere)
extern double mod1;   // expected range [-1..+1]
extern double mod2;   // expected range [-1..+1]

namespace {
  // ---- configurable defaults ----
  uint16_t g_attackMs = 120;
  uint16_t g_decayMs  = 700;
  uint8_t  g_minHpm   = 20;   // you said 20 → 40 range
  uint8_t  g_maxHpm   = 40;

  enum PanelId : uint8_t { BAR1, BAR2, BAR3, BAR4, MATRIX, STRIP1, STRIP2, PANEL_COUNT };

  struct Panel {
    uint8_t  level = 0;            // 0..255 envelope
    bool     rising = false;       // attack vs decay
    CRGB     color = CRGB::Black;
    uint32_t nextEventAt = 0;      // ms
    uint32_t lastMs = 0;           // ms
  };

  Panel P[PANEL_COUNT];

  // strip fill direction: true = start index → outward
  constexpr bool STRIP1_FILL_FROM_START = true;
  constexpr bool STRIP2_FILL_FROM_START = true;

  // --- helpers --------------------------------------------------------------

  inline uint32_t nowMs() { return millis(); }

  inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
  inline float normFromMod(double m) {
    return clamp01((float)((m + 1.0) * 0.5)); // [-1..+1] → [0..1]
  }
  inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

  inline uint8_t densityFromMod(double m) {
    const float t = normFromMod(m);
    return (uint8_t)(lerp((float)g_minHpm, (float)g_maxHpm, t) + 0.5f);
  }

  inline uint32_t scheduleNext(uint32_t tNow, uint8_t hitsPerMinute) {
    if (hitsPerMinute == 0) return tNow + 0x7FFFFFFF;
    const uint32_t meanMs = 60000UL / hitsPerMinute;
    const uint8_t u = random8();
    const float k = -logf((u + 1) / 256.0f);
    const uint32_t gap = (uint32_t)(meanMs * k);
    return tNow + gap;
  }

  inline void stepEnvelope(Panel& s, uint32_t dt) {
    if (s.rising) {
      const uint32_t inc = (uint32_t)255 * dt / (g_attackMs ? g_attackMs : 1);
      const uint16_t tmp = s.level + (uint16_t)min<uint32_t>(inc, 255);
      s.level = (tmp >= 255) ? 255 : (uint8_t)tmp;
      if (s.level == 255) s.rising = false;
    } else {
      const uint32_t dec = (uint32_t)255 * dt / (g_decayMs ? g_decayMs : 1);
      s.level = qsub8(s.level, (uint8_t)min<uint32_t>(dec, 255));
    }
  }

  // ---- drawing helpers -----------------------------------------------------

  // Light first `nOn` pixels, rest off. Reverses if fromStart=false.
  inline void drawStripFill(CRGB* leds, int count, int nOn, const CRGB& color, bool fromStart) {
    nOn = constrain(nOn, 0, count);
    if (fromStart) {
      for (int i = 0; i < nOn; ++i) leds[i] = color;
      for (int i = nOn; i < count; ++i) leds[i] = CRGB::Black;
    } else {
      int start = count - nOn;
      for (int i = 0; i < start; ++i) leds[i] = CRGB::Black;
      for (int i = start; i < count; ++i) leds[i] = color;
    }
  }

  inline CRGB scaled(const CRGB& c, uint8_t level) {
    return CRGB(scale8(c.r, level), scale8(c.g, level), scale8(c.b, level));
  }

  // --- dynamic density per panel -------------------------------------------
  inline uint8_t panelDensity(PanelId id) {
    switch (id) {
      case BAR1: case BAR2: case BAR3: case BAR4:
        return densityFromMod(mod1);                    // bars follow mod1
      case MATRIX:
        return densityFromMod(mod2);                    // matrix follows mod2
      case STRIP1: case STRIP2:
      default: {
        const float avg = 0.5f * (normFromMod(mod1) + normFromMod(mod2));
        const float d   = lerp((float)g_minHpm, (float)g_maxHpm, avg);
        return (uint8_t)(d + 0.5f);
      }
    }
  }

  inline void maybeFire(Panel& s, PanelId id, uint32_t tNow) {
    if (tNow >= s.nextEventAt && s.level < 32) {
      s.rising = true;
      s.color  = ColorFromPalette(currentPalette, random8(), 255, LINEARBLEND);
      if (s.level < 16) s.level = 16;
      s.nextEventAt = scheduleNext(tNow, panelDensity(id));
    }
  }

  // --- panel drawing logic --------------------------------------------------
  inline void drawPanel(PanelId id, const Panel& s) {
    // For strips: fill length based on envelope level (attack = grow, decay = shrink)
    if (id == STRIP1) {
      int nOn = (int)((s.level / 255.0f) * NUM_LEDS_STRIP + 0.5f);
      CRGB c  = scaled(s.color, 255); // constant brightness; use scaled(s.color, s.level) to fade
      drawStripFill(strip_1, NUM_LEDS_STRIP, nOn, c, STRIP1_FILL_FROM_START);
      return;
    }
    if (id == STRIP2) {
      int nOn = (int)((s.level / 255.0f) * NUM_LEDS_STRIP + 0.5f);
      CRGB c  = scaled(s.color, 255);
      drawStripFill(strip_2, NUM_LEDS_STRIP, nOn, c, STRIP2_FILL_FROM_START);
      return;
    }

    // Bars & matrix fill normally with fading brightness
    CRGB c = scaled(s.color, s.level);
    switch (id) {
      case BAR1:   fill_solid(bar_1,  NUM_LEDS_BAR,    c); break;
      case BAR2:   fill_solid(bar_2,  NUM_LEDS_BAR,    c); break;
      case BAR3:   fill_solid(bar_3,  NUM_LEDS_BAR,    c); break;
      case BAR4:   fill_solid(bar_4,  NUM_LEDS_BAR,    c); break;
      case MATRIX: fill_solid(matrix, NUM_LEDS_MATRIX, c); break;
      default: break;
    }
  }
}

// ----------------- public controls -----------------
void panelPulseSetTimes(uint16_t attackMs, uint16_t decayMs) {
  g_attackMs = attackMs;
  g_decayMs  = decayMs;
}

void panelPulseSetDensityRange(uint8_t minHpm, uint8_t maxHpm) {
  g_minHpm = minHpm;
  g_maxHpm = maxHpm < minHpm ? minHpm : maxHpm;
}

// ----------------- main pattern renderer -----------------
void renderPanelPulse() {
  // full clear (exclusive pattern)
  fill_solid(bar_1,  NUM_LEDS_BAR,    CRGB::Black);
  fill_solid(bar_2,  NUM_LEDS_BAR,    CRGB::Black);
  fill_solid(bar_3,  NUM_LEDS_BAR,    CRGB::Black);
  fill_solid(bar_4,  NUM_LEDS_BAR,    CRGB::Black);
  fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);
  fill_solid(strip_1,NUM_LEDS_STRIP,  CRGB::Black);
  fill_solid(strip_2,NUM_LEDS_STRIP,  CRGB::Black);

  const uint32_t tNow = nowMs();

  for (uint8_t id = 0; id < PANEL_COUNT; ++id) {
    Panel& s = P[id];
    const uint32_t dt = (s.lastMs == 0) ? 0 : (tNow - s.lastMs);
    s.lastMs = tNow;

    maybeFire(s, static_cast<PanelId>(id), tNow);
    stepEnvelope(s, dt);

    if (s.level) {
      drawPanel(static_cast<PanelId>(id), s);
    }
  }
}
