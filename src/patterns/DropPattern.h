#pragma once
#include <FastLED.h>

// --- Configuration (call in setup or live if you like) ---
// Spawn rates are in "drops per second" (approximate).
void dropPatternSetSpawnRates(float barsPerSec, float stripsPerSec, float matrixPerSec);

// Size ranges:
// - 1D sizes are in LED count (radius in pixels).
// - 2D sizes are in matrix radius (in "columns/pixels").
void dropPatternSetSizeRanges(uint8_t minRadius1D, uint8_t maxRadius1D,
                              uint8_t minRadius2D, uint8_t maxRadius2D);

// Optionally bias palette brightness for new drops (0..255, default 255)
void dropPatternSetPaletteBrightness(uint8_t v);

// --- Render (call once per frame). ---
// IMPORTANT: This does NOT clear the LEDs; it only draws new drops.
// Use your existing decay/fade elsewhere to make things trail & overlap nicely.
void renderDropPattern();

void dropPatternSetBarSizeBoost(float mul, uint8_t maxCap);
