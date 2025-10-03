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

void renderActivePattern() {
  switch(currentPattern) {
    case sinePattern:        render_sine(); break;
    case noisePattern:       render_noise(); break;
    case wavePattern:        renderWaves(); break;
    case searchlightPattern: renderSearchlightPattern(); break;
  };
}

void setup() {
  initServer();

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
  }

  //updateStrobeEffect();
  //updateEffects();

  FastLED.show();
}
