#pragma once
#include <FastLED.h>

// Configure attack/decay envelope (ms)
void panelPulseSetTimes(uint16_t attackMs, uint16_t decayMs);

// Configure modulation-based density mapping (hits/minute)
// mod in [-1..+1]  →  density in [min..max]
void panelPulseSetDensityRange(uint8_t minHpm, uint8_t maxHpm);

// Render the pattern (exclusive, clears and draws the frame)
void renderPanelPulse();
