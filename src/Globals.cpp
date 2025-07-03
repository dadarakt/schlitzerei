// Globals.cpp
#include "Globals.h"
#include "ColorPalettes.h"

// LED setup
CRGB strip_1[NUM_LEDS_STRIP];
CRGB strip_2[NUM_LEDS_STRIP];
CRGB bar_1[NUM_LEDS_BAR];
CRGB bar_2[NUM_LEDS_BAR];
CRGB matrix[NUM_LEDS_MATRIX];

int currentBrightness = 25;

CRGBPalette16 currentPalette = oranje; 
CRGBPalette16 targetPalette = oranje;
TBlendType currentBlending = LINEARBLEND;

// Modifiers

int t_mod = 0;
double mod1 = 0;
double mod2 = 0;
int decay_rate = 100;
bool autoCycle = false;

void updateModifiers() {
    t_mod = (millis() / 100) % 65535;
    mod1 = sin(t_mod * mod1_frequency);
    mod2 = sin(t_mod * mod2_frequency);
}