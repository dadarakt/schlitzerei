#pragma once
#include "FastLED.h"

void renderSearchlightPattern();
void updateSearchlight();
void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset);
void renderRedGlimmerMatrix();
void renderSearchlightCombined();