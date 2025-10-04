#include <FastLED.h>
#include "ColorPalettes.h"
#include "Globals.h"
#include "LEDHelpers.h"
#include "Server.h"
#include "StrobeEffect.h"
#include "Effects.h"

#include "patterns/Patterns.h"
#include "patterns/WaveRenderer.h"
#include "patterns/SineLoop.h"
#include "patterns/NoisePatterns.h"
#include "patterns/SearchlightPattern.h"
#include "MuxInput.h"

void renderActivePattern() {
  switch(currentPattern) {
    case sinePattern:        render_sine(); break;
    case noisePattern:       render_noise(); break;
    case wavePattern:        renderWaves(); break;
    case searchlightPattern: renderSearchlightPattern(); break;
  };
}

// simple edge-detect for button 0
bool lastBtn0 = false;
bool lastBtn1 = false;
bool lastBtn2 = false;
bool lastBtn3 = false;

void processInputs() {
  muxRead();

  // Pot → brightness (re-apply to FastLED)
  currentBrightness = map(gPotRaw, 0, 4095, 255, 0);
  FastLED.setBrightness(currentBrightness);   // <-- needed every update

  // Button 0: toggle on press edge
  bool b0 = gButtons[0];          // true when pressed (per your mux code)
  if (b0 && !lastBtn0) {
    nextPattern();
    //strobeActive = !strobeActive; // toggle once per press
  }
  lastBtn0 = b0;

  // Button 1: toggle on press edge
  bool b1 = gButtons[1];          // true when pressed (per your mux code)
  if (b1 && !lastBtn1) {
    nextPattern();
  }
  lastBtn1 = b1;

  // Button 0: toggle on press edge
  bool b2 = gButtons[2];          // true when pressed (per your mux code)
  if (b2 && !lastBtn2) {
    nextPattern();
    //nextPalette();
  }
  lastBtn2 = b2;

  // Button 0: toggle on press edge
  bool b3 = gButtons[3];          // true when pressed (per your mux code)
  if (b3 && !lastBtn3) {
    //nextPattern();
    strobeActive = !strobeActive; // toggle once per press
  }
  lastBtn3 = b3;
}

void setup() {
  initServer();
  muxInit();

  // LED setup
  FastLED.addLeds<WS2812, BAR1_DATA_PIN, GRB>(bar_1, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR2_DATA_PIN, GRB>(bar_2, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR3_DATA_PIN, GRB>(bar_3, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR4_DATA_PIN, GRB>(bar_4, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<WS2812, MATRIX_DATA_PIN, GRB>(matrix, NUM_LEDS_MATRIX).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<WS2801, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN, RGB>(strip_1, NUM_LEDS_STRIP);
  FastLED.addLeds<WS2801, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN, RGB>(strip_2, NUM_LEDS_STRIP);

  FastLED.setBrightness(currentBrightness);

  fill_solid(bar_1, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(bar_2, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(bar_3, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(bar_4, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(strip_1, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(strip_2, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);
}

void loop() {
  updateServer();

  EVERY_N_SECONDS(29) {
    if (autoCyclePatterns) {
      nextPattern();
    }
  }
  EVERY_N_SECONDS(17) {
    if (autoCyclePalettes) {
      nextPalette();
    }
  }

  EVERY_N_MILLISECONDS(1000 * (1 / fps)) {
    updateModifiers();
    updatePalettes();

    fade_all(decay_rate);

    renderActivePattern();
    processInputs();
  }

  updateStrobeEffect();
  updateEffects();

  FastLED.show();
}

