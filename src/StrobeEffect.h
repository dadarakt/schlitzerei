#pragma once
#include <FastLED.h>

void startStrobeContinuous(bool useWhite = true);
void stopStrobe();
void updateStrobeEffect();
bool isStrobeActive();
