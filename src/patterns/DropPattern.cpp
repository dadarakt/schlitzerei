#include "DropPattern.h"
#include "Globals.h"
#include "ColorPalettes.h"
#include "LEDHelpers.h"

using namespace LEDStore;

// Provided elsewhere in your project
extern CRGBPalette16 currentPalette;

// If you want modulators to influence rates/sizes later, you can extern them:
extern double mod1; // e.g., drive 1D spawn rate/size
extern double mod2; // e.g., drive 2D spawn rate/size

namespace {

// -------------------- Tunables / State --------------------
float  g_spawnBarsPerSec   = 0.8f;  // average drops/sec across ALL bars
float  g_spawnStripsPerSec = 0.6f;  // average drops/sec across BOTH strips
float  g_spawnMatrixPerSec = 1.0f;  // average drops/sec on the matrix

uint8_t g_minR1D = 2;   // min radius (LEDs) for 1D blobs
uint8_t g_maxR1D = 10;  // max radius (LEDs) for 1D blobs

uint8_t g_minR2D = 2;   // min radius (pixels) for 2D blobs
uint8_t g_maxR2D = 6;   // max radius (pixels) for 2D blobs

uint8_t g_paletteBrightness = 255; // 0..255 per-drop base brightness

// Helper: simple Poisson-ish sampler: expected 'lambda' events per call (per frame).
inline uint8_t samplePoisson(float lambda) {
  // For small lambda, at most a few events. We'll use a simple looped Bernoulli approximation:
  // If lambda < 1, one trial with p=lambda; if >1, multiple trials with p=fractional part.
  uint8_t k = 0;
  while (lambda >= 1.0f) {
    if (random8() < 255) ++k;  // always success; count down whole part
    lambda -= 1.0f;
  }
  if (lambda > 0.0f) {
    if (random8() < (uint8_t)(lambda * 255.0f)) ++k;
  }
  return k;
}

// Scale & add color (saturating)
inline void addColorScaled(CRGB& dst, const CRGB& c, uint8_t scale) {
  if (scale == 0) return;
  CRGB s( scale8(c.r, scale), scale8(c.g, scale), scale8(c.b, scale) );
  dst.r = qadd8(dst.r, s.r);
  dst.g = qadd8(dst.g, s.g);
  dst.b = qadd8(dst.b, s.b);
}

// -------------------- 1D drawing --------------------
inline void drawBlob1D(CRGB* leds, int count, int center, uint8_t radius, const CRGB& color) {
  // Linear falloff from center to edges, squared for softer look
  const int start = max(0, center - (int)radius);
  const int end   = min(count - 1, center + (int)radius);
  for (int i = start; i <= end; ++i) {
    int d = abs(i - center);
    uint8_t w = (radius == 0) ? 255 : (uint8_t)((255 * (radius - d)) / (radius)); // 0..255
    // soften by squaring
    w = scale8(w, w);
    addColorScaled(leds[i], color, w);
  }
}

// -------------------- 2D drawing (matrix) --------------------
inline void drawBlob2D(uint8_t cx, uint8_t cy, uint8_t radius, const CRGB& color) {
  const int xmin = max<int>(0,  cx - radius);
  const int xmax = min<int>(COLS - 1, cx + radius);
  const int ymin = max<int>(0,  cy - radius);
  const int ymax = min<int>(ROWS - 1, cy + radius);

  for (int x = xmin; x <= xmax; ++x) {
    for (int y = ymin; y <= ymax; ++y) {
      int dx = x - cx;
      int dy = y - cy;
      int dist2 = dx * dx + dy * dy;
      int r2 = radius * radius;
      if (dist2 <= r2) {
        // weight ~ (1 - d/r), squared for softer edge
        // Use integer approx: w = 255 * (1 - sqrt(dist2)/radius)
        // We'll avoid sqrt: use linear approximation via manhattan-like falloff
        // Or compute a simple linear using hypotf for nicer look (ok on ESP32):
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

// -------------------- Spawners --------------------
inline void spawnBarDrop(CRGB* bar, int count) {
  const int center = random8(count); // 0..count-1
  const uint8_t rad = random8(g_minR1D, (uint8_t)(g_maxR1D + 1));
  const uint8_t idx = random8();
  const CRGB col   = ColorFromPalette(currentPalette, idx, g_paletteBrightness, LINEARBLEND);
  drawBlob1D(bar, count, center, rad, col);
}

inline void spawnStripDrop(CRGB* strip, int count) {
  const int center = random8(count);
  const uint8_t rad = random8(g_minR1D, (uint8_t)(g_maxR1D + 1));
  const uint8_t idx = random8();
  const CRGB col   = ColorFromPalette(currentPalette, idx, g_paletteBrightness, LINEARBLEND);
  drawBlob1D(strip, count, center, rad, col);
}

inline void spawnMatrixDrop() {
  const uint8_t cx  = random8(COLS);
  const uint8_t cy  = random8(ROWS);
  const uint8_t rad = random8(g_minR2D, (uint8_t)(g_maxR2D + 1));
  const uint8_t idx = random8();
  const CRGB col   = ColorFromPalette(currentPalette, idx, g_paletteBrightness, LINEARBLEND);
  drawBlob2D(cx, cy, rad, col);
}

} // namespace

// -------------------- Public API --------------------
void dropPatternSetSpawnRates(float barsPerSec, float stripsPerSec, float matrixPerSec) {
  g_spawnBarsPerSec   = max(0.0f, barsPerSec);
  g_spawnStripsPerSec = max(0.0f, stripsPerSec);
  g_spawnMatrixPerSec = max(0.0f, matrixPerSec);
}

void dropPatternSetSizeRanges(uint8_t minRadius1D, uint8_t maxRadius1D,
                              uint8_t minRadius2D, uint8_t maxRadius2D) {
  if (maxRadius1D < minRadius1D) maxRadius1D = minRadius1D;
  if (maxRadius2D < minRadius2D) maxRadius2D = minRadius2D;
  g_minR1D = minRadius1D;
  g_maxR1D = maxRadius1D;
  g_minR2D = minRadius2D;
  g_maxR2D = maxRadius2D;
}

void dropPatternSetPaletteBrightness(uint8_t v) {
  g_paletteBrightness = v;
}

// Draw new drops (do NOT clear; let your global fade handle decay/overlap)
void renderDropPattern() {
  // Estimate drops to spawn this frame from per-second rates and frame time.
  static uint32_t lastMs = 0;
  const uint32_t now = millis();
  const uint32_t dt  = (lastMs == 0) ? 16 : (now - lastMs);
  lastMs = now;

  // Convert to expected count per frame
  const float seconds = dt / 1000.0f;
  const float expBars   = g_spawnBarsPerSec   * seconds;
  const float expStrips = g_spawnStripsPerSec * seconds;
  const float expMatrix = g_spawnMatrixPerSec * seconds;

  // Sample number of events (approximate)
  uint8_t nBars   = samplePoisson(expBars);
  uint8_t nStrips = samplePoisson(expStrips);
  uint8_t nMatrix = samplePoisson(expMatrix);

  // Distribute bar events across the four bars (uniform)
  while (nBars--) {
    switch (random8(4)) {
      case 0: spawnBarDrop(bar_1, NUM_LEDS_BAR); break;
      case 1: spawnBarDrop(bar_2, NUM_LEDS_BAR); break;
      case 2: spawnBarDrop(bar_3, NUM_LEDS_BAR); break;
      default: spawnBarDrop(bar_4, NUM_LEDS_BAR); break;
    }
  }

  // Distribute strip events across the two strips
  while (nStrips--) {
    if (random8() & 1) spawnStripDrop(strip_1, NUM_LEDS_STRIP);
    else               spawnStripDrop(strip_2, NUM_LEDS_STRIP);
  }

  // Spawn matrix drops
  while (nMatrix--) {
    spawnMatrixDrop();
  }
}
