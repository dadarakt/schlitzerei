// Globals.cpp
#include "Globals.h"
#include "ColorPalettes.h"
#include "Modes.h"

// Define the actual storage for the LED buffers
CRGB strip_1[NUM_LEDS_STRIP];
CRGB strip_2[NUM_LEDS_STRIP];
CRGB bar_1[NUM_LEDS_BAR];
CRGB bar_2[NUM_LEDS_BAR];
CRGB matrix[NUM_LEDS_MATRIX];

double mod1 = 0;
double mod2 = 0;
Mode current_mode = center_pulse;
int currentBrightness = 50;
CRGBPalette16 currentPalette = oranje; 