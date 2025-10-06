#include "DropPattern.h"
#include "Globals.h"
#include "ColorPalettes.h"
#include "LEDHelpers.h"   // XY()

using namespace LEDStore;

extern CRGBPalette16 currentPalette;
extern double mod1;   // drive bars/strips rate
extern double mod2;   // drive matrix rate

namespace {

// -------------------- Tunables --------------------
// Higher default base rates (drops per second)
float  g_baseBarsPerSec   = 2.5f;   // across ALL bars combined
float  g_baseStripsPerSec = 1.5f;   // across BOTH strips combined
float  g_baseMatrixPerSec = 6.0f;   // across the matrix

// Mod multipliers: norm(mod) in [0..1] -> rate *= lerp(min, max)
float  g_modMinMul = 0.5f;
float  g_modMaxMul = 2.0f;

// Bars-only radius boost (affects variance & average size)
float   g_barRadiusMul = 4.0f;   // >1 = bigger blobs on bars
uint8_t g_barRadiusCap = 80;     // hard cap so they don't get huge

// Sizes
uint8_t g_minR1D = 3;
uint8_t g_maxR1D = 12;
uint8_t g_minR2D = 3;
uint8_t g_maxR2D = 8;

uint8_t g_paletteBrightness = 230; // headroom for other overlays

// Accumulators (rate * dt)
float g_accBars   = 0.0f;
float g_accStrips = 0.0f;
float g_accMatrix = 0.0f;

// Helpers ---------------------------------------------------
inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
inline float normFromMod(double m) { return clamp01((float)((m + 1.0) * 0.5)); }
inline float lerpF(float a, float b, float t) { return a + (b - a) * t; }

// Scale & add color (saturating)
inline void addColorScaled(CRGB& dst, const CRGB& c, uint8_t scale) {
  if (!scale) return;
  CRGB s(scale8(c.r, scale), scale8(c.g, scale), scale8(c.b, scale));
  dst.r = qadd8(dst.r, s.r);
  dst.g = qadd8(dst.g, s.g);
  dst.b = qadd8(dst.b, s.b);
}

// 1D blob (soft)
inline void drawBlob1D(CRGB* leds, int count, int center, uint8_t radius, const CRGB& color) {
  const int start = max(0, center - (int)radius);
  const int end   = min(count - 1, center + (int)radius);
  for (int i = start; i <= end; ++i) {
    int d = abs(i - center);
    uint8_t w = (radius == 0) ? 255 : (uint8_t)((255 * (radius - d)) / (radius)); // 0..255
    w = scale8(w, w); // soften
    addColorScaled(leds[i], color, w);
  }
}

// 2D blob (soft)
inline void drawBlob2D(uint8_t cx, uint8_t cy, uint8_t radius, const CRGB& color) {
  const int xmin = max<int>(0,  cx - radius);
  const int xmax = min<int>(COLS - 1, cx + radius);
  const int ymin = max<int>(0,  cy - radius);
  const int ymax = min<int>(ROWS - 1, cy + radius);

  for (int x = xmin; x <= xmax; ++x) {
    for (int y = ymin; y <= ymax; ++y) {
      int dx = x - cx, dy = y - cy;
      int dist2 = dx*dx + dy*dy;
      int r2 = radius*radius;
      if (dist2 <= r2) {
        float d = sqrtf((float)dist2);
        float lin = 1.0f - (d / (float)radius);
        if (lin < 0) lin = 0;
        uint8_t w = (uint8_t)(lin * 255.0f + 0.5f);
        w = scale8(w, w); // soften
        addColorScaled(matrix[XY(x, y)], color, w);
      }
    }
  }
}

// Spawners -----------------------------------------
inline CRGB randomPaletteColor() {
  return ColorFromPalette(currentPalette, random8(), g_paletteBrightness, LINEARBLEND);
}

inline void spawnBarDrop(CRGB* bar, int count) {
  const int center   = random8(count);
  const uint8_t base = random8(g_minR1D, (uint8_t)(g_maxR1D + 1));

  // Scale radius for bars only, and clamp to a sensible cap
  int scaled = (int)lroundf(base * g_barRadiusMul);
  if (scaled < (int)g_minR1D)  scaled = g_minR1D;        // don’t get smaller than base min
  if (scaled > (int)g_barRadiusCap) scaled = g_barRadiusCap;

  drawBlob1D(bar, count, center, (uint8_t)scaled, randomPaletteColor());
}

inline void spawnStripDrop(CRGB* strip, int count) {
  const int center = random8(count);
  const uint8_t rad = random8(g_minR1D, (uint8_t)(g_maxR1D + 1));
  drawBlob1D(strip, count, center, rad, randomPaletteColor());
}

inline void spawnMatrixDrop() {
  const uint8_t cx  = random8(COLS);
  const uint8_t cy  = random8(ROWS);
  const uint8_t rad = random8(g_minR2D, (uint8_t)(g_maxR2D + 1));
  drawBlob2D(cx, cy, rad, randomPaletteColor());
}

} // namespace

// -------------------- Public API --------------------
void dropPatternSetSpawnRates(float barsPerSec, float stripsPerSec, float matrixPerSec) {
  g_baseBarsPerSec   = max(0.0f, barsPerSec);
  g_baseStripsPerSec = max(0.0f, stripsPerSec);
  g_baseMatrixPerSec = max(0.0f, matrixPerSec);
}

void dropPatternSetSizeRanges(uint8_t minRadius1D, uint8_t maxRadius1D,
                              uint8_t minRadius2D, uint8_t maxRadius2D) {
  if (maxRadius1D < minRadius1D) maxRadius1D = minRadius1D;
  if (maxRadius2D < minRadius2D) maxRadius2D = minRadius2D;
  g_minR1D = minRadius1D; g_maxR1D = maxRadius1D;
  g_minR2D = minRadius2D; g_maxR2D = maxRadius2D;
}

void dropPatternSetPaletteBrightness(uint8_t v) {
  g_paletteBrightness = v;
}

void dropPatternSetBarSizeBoost(float mul, uint8_t maxCap) {
  g_barRadiusMul = (mul < 0.1f) ? 0.1f : mul;
  g_barRadiusCap = maxCap;
}

// -------------------- Render --------------------
// NOTE: do NOT clear; rely on your global fade elsewhere.
void renderDropPattern() {
  static uint32_t lastMs = 0;
  const uint32_t now = millis();
  uint32_t dt = (lastMs == 0) ? 16 : (now - lastMs);
  lastMs = now;

  // Clamp pathological dt (e.g., after pause) to keep burst sane
  if (dt > 80) dt = 80;

  const float seconds = dt / 1000.0f;

  // Live mod multipliers
  const float mulBars   = lerpF(g_modMinMul, g_modMaxMul, normFromMod(mod1));
  const float mulStrips = lerpF(g_modMinMul, g_modMaxMul, normFromMod(mod1));
  const float mulMatrix = lerpF(g_modMinMul, g_modMaxMul, normFromMod(mod2));

  // Effective per-second rates this frame
  const float rateBars   = g_baseBarsPerSec   * mulBars;
  const float rateStrips = g_baseStripsPerSec * mulStrips;
  const float rateMatrix = g_baseMatrixPerSec * mulMatrix;

  // Accumulate expected events
  g_accBars   += rateBars   * seconds;
  g_accStrips += rateStrips * seconds;
  g_accMatrix += rateMatrix * seconds;

  // Spawn whole events
  while (g_accBars >= 1.0f)   { switch (random8(4)) {
                                  case 0: spawnBarDrop(bar_1, NUM_LEDS_BAR); break;
                                  case 1: spawnBarDrop(bar_2, NUM_LEDS_BAR); break;
                                  case 2: spawnBarDrop(bar_3, NUM_LEDS_BAR); break;
                                  default:spawnBarDrop(bar_4, NUM_LEDS_BAR); break;
                                } g_accBars -= 1.0f; }
  while (g_accStrips >= 1.0f) { if (random8() & 1) spawnStripDrop(strip_1, NUM_LEDS_STRIP);
                                else               spawnStripDrop(strip_2, NUM_LEDS_STRIP);
                                g_accStrips -= 1.0f; }
  while (g_accMatrix >= 1.0f) { spawnMatrixDrop(); g_accMatrix -= 1.0f; }

  // Fractional chance for one extra spawn per group (keeps it lively)
  if (g_accBars > 0.0f && random8() < (uint8_t)(g_accBars * 255.0f)) {
    switch (random8(4)) {
      case 0: spawnBarDrop(bar_1, NUM_LEDS_BAR); break;
      case 1: spawnBarDrop(bar_2, NUM_LEDS_BAR); break;
      case 2: spawnBarDrop(bar_3, NUM_LEDS_BAR); break;
      default:spawnBarDrop(bar_4, NUM_LEDS_BAR); break;
    }
    g_accBars = 0.0f;
  }
  if (g_accStrips > 0.0f && random8() < (uint8_t)(g_accStrips * 255.0f)) {
    if (random8() & 1) spawnStripDrop(strip_1, NUM_LEDS_STRIP);
    else               spawnStripDrop(strip_2, NUM_LEDS_STRIP);
    g_accStrips = 0.0f;
  }
  if (g_accMatrix > 0.0f && random8() < (uint8_t)(g_accMatrix * 255.0f)) {
    spawnMatrixDrop();
    g_accMatrix = 0.0f;
  }
}
