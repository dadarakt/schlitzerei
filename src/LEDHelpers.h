// LEDHelpers.h
#ifndef LED_HELPERS_H
#define LED_HELPERS_H
#include "FastLED.h"
//#include <Arduino.h>

void fill_solid_all(CRGB color);

void fade_leds(CRGB* leds, int num_leds, int fade_amount);
void fade_all(int fade_amount);

void add_solid(CRGB* leds, int num_leds, CRGB color);
void add_solid_all(CRGB color);

int XY(uint8_t x, uint8_t y);

#endif