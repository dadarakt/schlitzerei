#include <FastLED.h>
#include "ColorPalettes.h"
#include "Globals.h"
#include "LEDHelpers.h"
//#include "Server.h"
#include "StrobeEffect.h"
#include "Effects.h"
#include "MuxInput.h"
#include "CrowdBlinder.h"

#include "patterns/Patterns.h"
#include "patterns/WaveRenderer.h"
#include "patterns/SineLoop.h"
#include "patterns/NoisePatterns.h"
#include "patterns/SearchlightPattern.h"
#include "patterns/PanelPulse.h"
#include "patterns/DropPattern.h"

using namespace LEDStore;

void renderActivePattern() {
  switch(currentPattern) {
    case dropPattern:        renderDropPattern(); break;
    case panelPulsePattern:  renderPanelPulse(); break;
    case sinePattern:        render_sine(); break;
    case noisePattern:       render_noise(); break;
    case wavePattern:        renderWaves(); break;
    case searchlightPattern: renderSearchlightPattern(); break;
  };
}

void setup() {
  //initServer();
  muxInit();
  FastLED.clear();

  FastLED.addLeds<WS2812, BAR_1_DATA_PIN, GRB>(bar_1, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR_2_DATA_PIN, GRB>(bar_2, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR_3_DATA_PIN, GRB>(bar_3, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR_4_DATA_PIN, GRB>(bar_4, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);

  FastLED
    .addLeds<WS2812, MATRIX_DATA_PIN, GRB>(matrix, NUM_LEDS_MATRIX)
    .setCorrection(TypicalLEDStrip);

  FastLED.addLeds<WS2801, STRIP_1_DATA_PIN, STRIP_1_CLOCK_PIN, RGB>(strip_1, NUM_LEDS_STRIP);
  FastLED.addLeds<WS2801, STRIP_2_DATA_PIN, STRIP_2_CLOCK_PIN, RGB>(strip_2, NUM_LEDS_STRIP);
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
  //updateServer();

  EVERY_N_SECONDS(177) { // 111
    if (autoCyclePatterns) {
      nextPattern();
    }
  }
  EVERY_N_SECONDS(111) { // 51
    if (autoCyclePalettes) {
      nextPalette();
    }
  }

  EVERY_N_MILLISECONDS(1000 * (1 / fps)) {
    updateModifiers();
    updatePalettes();

    fade_all(decay_rate);

    renderActivePattern();
    muxRead();
    updateStrobeEffect();
    crowdBlinderUpdate();
    //updateEffects();

    FastLED.show();
  }
}

