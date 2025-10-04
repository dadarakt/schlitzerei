#pragma once
#include <FastLED.h>

void triggerStrobeBurst(unsigned long durationMs, bool useWhite = true);
void stopStrobe();
void updateStrobeEffect();
bool isStrobeActive();