// StrobeEffect.h
#pragma once

#include <FastLED.h>

// Call this to start a strobe
void startStrobe(unsigned long durationMs = 1000, bool useWhite = false);

// Call this regularly in the render loop
void updateStrobeEffect();

// Optional: Stop strobe early
void stopStrobe();

// Query if currently active
bool isStrobeActive();