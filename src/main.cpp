#include <FastLED.h>
#include "ColorPalettes.h"
#include "Modes.h"
#include "Globals.h"
#include "LEDHelpers.h"
#include "Server.h"

#include "patterns/WaveRenderer.h"
#include "patterns/SineLoop.h"
#include "patterns/NoisePatterns.h"
#include "patterns/SearchlightPattern.h"

unsigned long now;

bool long_pulse = true;
unsigned long last_pulse = 0;
const unsigned long short_interval = 1000;
const unsigned long long_interval = 4000;
const unsigned long pulse_rise = 200;
const unsigned long pulse_fall = 1000;
const unsigned long pulse_duration = pulse_rise + pulse_fall;

bool paletteFlipState = false; // false = A, true = B
bool aboveThreshold = false;

bool particle_event = false;
unsigned long last_particle_event = 0;
unsigned long event_duration = 0;
unsigned long event_intensity = 0;
unsigned long next_particle_event = 0;

void determine_next_particle_event(long now) {
  next_particle_event = now + random(60, 120) * 1000;
  event_duration = random(5, 20) * 1000;
  event_intensity = random(3, 15);
}

void check_particle_event() {
  now = millis();
  if (!particle_event && now > next_particle_event) {
    particle_event = true;
  } else if (particle_event && now > (next_particle_event + event_duration)) {
    particle_event = false;
    determine_next_particle_event(now);
  }
}

void add_glitter(uint8_t num_particles)
{
  for(int p = 0; p < num_particles; p++) {
    bar_1[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    bar_2[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    strip_1[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
    strip_2[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
  }
}

void add_pulse() {
  now = millis();
  if (long_pulse && now > (last_pulse + long_interval)) {
    last_pulse = now;
    long_pulse = !long_pulse;
    return;
  }

  if (!long_pulse && now > (last_pulse + short_interval)) {
    last_pulse = now;
    long_pulse = !long_pulse;
    return;
  }

  if (now < (last_pulse + pulse_duration)) {
    unsigned long t_pulse = now - last_pulse;

    if (t_pulse <= pulse_rise) {
       float amount = t_pulse / (float)pulse_rise;
       add_solid_all(CRGB(amount * 255, amount * 50, amount * 50));
       return;
    }
  }
}

void setup() {
  initServer();
  determine_next_particle_event(0);

  // LED setup
  FastLED.addLeds<WS2812, BAR1_DATA_PIN, GRB>(bar_1, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR2_DATA_PIN, GRB>(bar_2, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, MATRIX_DATA_PIN, GRB>(matrix, NUM_LEDS_MATRIX).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2801, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN, RGB>(strip_1, NUM_LEDS_STRIP);
  FastLED.addLeds<WS2801, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN, RGB>(strip_2, NUM_LEDS_STRIP);
  FastLED.setBrightness(currentBrightness);

  fill_solid(bar_1, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(bar_2, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(strip_1, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(strip_2, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);

  initSearchlight();

  FastLED.show();
}

void loop() {
  updateServer();

  EVERY_N_MILLISECONDS(1000 * (1 / fps)) {
    updateModifiers();
    updatePalettes();

    fade_all(decay_rate);

    //updateSearchlight();
    // renderSearchlightPattern();
   

    if (current_mode == sines) {
      render_sine();
    } else if (current_mode == center_pulse) {
      renderWaves();
    } else {
      render_noise();
    }

    FastLED.show();
  }

  //check_particle_event();
  //if (particle_event) {
  //  add_glitter(event_intensity);
  //}

  //if (current_mode == pulsing) {
  //  add_pulse();
  //}
}
