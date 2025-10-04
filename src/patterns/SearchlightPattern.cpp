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

// Palette-driven, high-contrast glimmer for 1D arrays (bars / strips)
void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset = 0) {
  extern CRGBPalette16 currentPalette;

  // Faster evolution; mod1 still nudges speed
  const uint32_t tFast = millis();
  const uint16_t tNoise = (uint16_t)(tFast * (0.020f + 0.020f * ((mod1 + 1.0) * 0.5))); // ~20–40 Hz

  // Global palette scroll so hues drift over time
  const uint8_t scroll = (uint8_t)(tFast >> 5); // slow index drift

  for (int i = 0; i < num_leds; ++i) {
    // Spatial scale: bigger step = more variation LED-to-LED
    const uint16_t x = i * 13 + offset;

    // Brightness texture (noise + tiny ripple)
    uint8_t n  = inoise8(x, tNoise);
    uint8_t hf = sin8((i * 7) + (tFast >> 2));   // shimmer

    uint16_t v16 = (uint16_t)n + (hf >> 2);
    if (v16 > 255) v16 = 255;

    // Contrast-y brightness with headroom
    uint8_t val = sqrt16((uint16_t)(v16 * 257U)); // gamma-ish
    val = scale8(val, 220);                       // cap for overlay headroom

    // Palette index: base noise + spatial gradient + slow scroll
    // This guarantees visible hue differences along the strip
    uint8_t idx = n + (i * 3) + scroll;

    // Pull color from current palette at 'idx' with brightness 'val'
    leds[i] = ColorFromPalette(currentPalette, idx, val, LINEARBLEND);
  }
}

// Palette-driven, high-contrast glimmer for the matrix
void renderRedGlimmerMatrix() {
  extern CRGBPalette16 currentPalette;

  const uint32_t tFast = millis();
  const uint16_t tNoise = (uint16_t)(tFast * (0.018f + 0.022f * ((mod1 + 1.0) * 0.5)));
  const uint8_t  scroll = (uint8_t)(tFast >> 5);

  for (uint8_t x = 0; x < COLS; ++x) {
    for (uint8_t y = 0; y < ROWS; ++y) {
      // Slightly different spatial scale in X/Y so colors vary across the matrix
      const uint16_t nx = x * 17;
      const uint16_t ny = y * 19;

      uint8_t n  = inoise8(nx, ny, tNoise);
      uint8_t hf = sin8((x * 9 + y * 11) + (tFast >> 2));

      uint16_t v16 = (uint16_t)n + (hf >> 3); // lighter ripple on matrix
      if (v16 > 255) v16 = 255;

      uint8_t val = sqrt16((uint16_t)(v16 * 257U));
      val = scale8(val, 220);

      // Palette index varies across space and time (diagonal bias + scroll)
      uint8_t idx = n + (x * 2) - (y * 2) + scroll;

      matrix[XY(x, y)] = ColorFromPalette(currentPalette, idx, val, LINEARBLEND);
    }
  }
}


// --- Sweep helpers ----------------------------------------------------------

static inline void addWhiteToAll(CRGB* leds, int count, uint8_t v) {
  if (!v) return;
  CRGB w(v, v, v);
  for (int i = 0; i < count; ++i) leds[i] += w;
}

static inline void applySweepToStrip(CRGB* leds, int count, float base, float pos, float width, bool reverse) {
  for (int i = 0; i < count; ++i) {
    float localIndex = reverse ? (count - 1 - i) : i;
    float gpos = base + localIndex;
    float brightness = max(0.0f, 1.0f - fabsf(gpos - pos) / width);
    uint8_t v = (uint8_t)(brightness * 255.0f + 0.5f);
    leds[i] += CRGB(v, v, v);
  }
}

static inline void applySweepToMatrix(float base, float pos, float width) {
  for (uint8_t x = 0; x < COLS; ++x) {
    float gpos = base + x;
    float brightness = max(0.0f, 1.0f - fabsf(gpos - pos) / width);
    uint8_t v = (uint8_t)(brightness * 255.0f + 0.5f);
    CRGB w(v, v, v);
    for (uint8_t y = 0; y < ROWS; ++y) {
      matrix[XY(x, y)] += w;
    }
  }
}

// Treat a whole vertical bar as ONE horizontal pixel in the sweep
static inline void applySweepToBarPixel(CRGB* bar, int count, float barBase, float pos, float width) {
  float brightness = max(0.0f, 1.0f - fabsf(barBase - pos) / width);
  uint8_t v = (uint8_t)(brightness * 255.0f + 0.5f);
  addWhiteToAll(bar, count, v);
}

// --- Combined sweep ---------------------------------------------------------

void renderSearchlightCombined() {
  if (!searchlight_active) return;

  // Horizontal layout (left → right):
  // strip_1 → bar_1 (1px) → bar_2 (1px) → matrix (COLS px) → bar_3 (1px) → bar_4 (1px) → strip_2
  float base = 0.0f;
  const float off_strip1 = base;                    base += NUM_LEDS_STRIP;
  const float off_bar1   = base;                    base += 1.0f;        // one pixel
  const float off_bar2   = base;                    base += 1.0f;        // one pixel
  const float off_matrix = base;                    base += COLS;
  const float off_bar3   = base;                    base += 1.0f;        // one pixel
  const float off_bar4   = base;                    base += 1.0f;        // one pixel
  const float off_strip2 = base;                    base += NUM_LEDS_STRIP;
  const float total_width = base;

  // Beam width adjustment: medium-wide
  float oscillation = sinf(millis() / 1000.0f) * 0.5f + 0.5f; // 0..1
  float pos   = oscillation * total_width;

  // Previously: 12..30   |   Before that: 6..16
  // Now: ~8..22 — a balanced width
  float width = 8.0f + ((mod2 + 1.0f) * 0.5f) * 14.0f;


  // Left elements drawn reversed; center/right forward
  applySweepToStrip (strip_1, NUM_LEDS_STRIP, off_strip1, pos, width, /*reverse=*/true);
  applySweepToBarPixel(bar_1,  NUM_LEDS_BAR,  off_bar1,   pos, width);
  applySweepToBarPixel(bar_2,  NUM_LEDS_BAR,  off_bar2,   pos, width);

  applySweepToMatrix(off_matrix, pos, width);

  applySweepToBarPixel(bar_3,  NUM_LEDS_BAR,  off_bar3,   pos, width);
  applySweepToBarPixel(bar_4,  NUM_LEDS_BAR,  off_bar4,   pos, width);
  applySweepToStrip (strip_2, NUM_LEDS_STRIP, off_strip2, pos, width, /*reverse=*/false);
}

// --- Public render ----------------------------------------------------------

void renderSearchlightPattern() {
  updateSearchlight();

  // Base red “glimmer” on each panel
  renderRedGlimmer(bar_1,  NUM_LEDS_BAR);
  renderRedGlimmer(bar_2,  NUM_LEDS_BAR, 1000);
  renderRedGlimmer(bar_3,  NUM_LEDS_BAR, 2000);
  renderRedGlimmer(bar_4,  NUM_LEDS_BAR, 3000);

  renderRedGlimmer(strip_1, NUM_LEDS_STRIP, 1000);
  renderRedGlimmer(strip_2, NUM_LEDS_STRIP);

  renderRedGlimmerMatrix();

  // Add the moving searchlight sweep on top
  renderSearchlightCombined();
}
