#pragma once

#include <FastLED.h>

// --- LED Matrix ---
#define MATRIX_LED_TYPE WS2812
constexpr int COLS = 32;
constexpr int ROWS = 8;
constexpr int NUM_LEDS_MATRIX = COLS * ROWS;
constexpr int MATRIX_MAX_DIMENSION = (COLS > ROWS ? COLS : ROWS);
constexpr int MATRIX_DATA_PIN = 2;

// --- LED Bars ---
// 1 left outer
// 2 left inner
// 3 right inner
// 4 right outer
#define BAR_LED_TYPE WS2812
constexpr int NUM_LEDS_BAR = 142;
constexpr int BAR1_DATA_PIN = 22; // 22
constexpr int BAR2_DATA_PIN = 23; // 23
constexpr int BAR3_DATA_PIN = 4;  
constexpr int BAR4_DATA_PIN = 15;  

// --- LED Strips ---
// 1 left
// 2 right
#define STRIP_LED_TYPE WS2801
constexpr int NUM_LEDS_STRIP = 50;
constexpr int STRIP1_CLOCK_PIN = 21;
constexpr int STRIP1_DATA_PIN = 19;
constexpr int STRIP2_CLOCK_PIN = 18;
constexpr int STRIP2_DATA_PIN = 5;

// --- FastLED CRGB arrays (extern) ---
extern CRGB matrix[NUM_LEDS_MATRIX];
extern CRGB bar_1[NUM_LEDS_BAR];
extern CRGB bar_2[NUM_LEDS_BAR];
extern CRGB bar_3[NUM_LEDS_BAR];
extern CRGB bar_4[NUM_LEDS_BAR];
extern CRGB strip_1[NUM_LEDS_STRIP];
extern CRGB strip_2[NUM_LEDS_STRIP];

// Constants
constexpr float fps = 120;
constexpr double mod1_frequency = 0.005;
constexpr double mod2_frequency = 0.008;

// Variables
extern double mod1;
extern double mod2;
extern int currentBrightness;
extern TBlendType currentBlending;
extern int decay_rate;
extern bool autoCyclePatterns;
extern bool autoCyclePalettes;
extern bool strobeActive;
extern bool particleEffectActive;
extern bool glitterEffectActive;
extern bool pulseEffectActive;

extern CRGBPalette16 currentPalette; 
extern CRGBPalette16 targetPalette;

extern void updateModifiers();