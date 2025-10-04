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

void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset = 0) {
  float speed = 25 + ((mod1 + 1.0f) * 0.5f) * 4.0f;
  for (int i = 0; i < num_leds; i++) {
    uint8_t noise = inoise8(i * 10, millis() / 8 + offset);
    int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) * 0.5f) * 80.0f;
    uint8_t red = min(red_intensity, 180);
    leds[i] = CRGB(red, red / 10, red / 20);
  }
}

void renderRedGlimmerMatrix() {
  float speed = 25 + ((mod1 + 1.0f) * 0.5f) * 8.0f;
  for (uint8_t x = 0; x < COLS; x++) {
    for (uint8_t y = 0; y < ROWS; y++) {
      uint8_t noise = inoise8(x * 15, y * 15, millis() / 10);
      int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) * 0.5f) * 80.0f;
      uint8_t red = min(red_intensity, 180);
      matrix[XY(x, y)] = CRGB(red, red / 10, red / 20);
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
