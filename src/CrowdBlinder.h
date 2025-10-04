#pragma once
#include <FastLED.h>

// Configure the envelope (ms)
void crowdBlinderSetAttackRelease(uint16_t attackMs, uint16_t releaseMs);

// While-held control
void crowdBlinderEnable(bool enable);

// Call each frame (AFTER your normal rendering, BEFORE FastLED.show())
void crowdBlinderUpdate();

// Optional: current level (0..255)
uint8_t crowdBlinderLevel();
