#include "SineLoop.h"
#include "Globals.h"

// moving sine waves
byte intense;
fract8 intensity = 255;
int speed_1 = 9;
int speed_2 = 5;
int speed_3 = 20;
int frequency_1 = 2;
int frequency_2 = 5;
int frequency_3 = 4;

int i, j, t;

void sine_loop(CRGB leds[], int num_leds, int factor) {
  long now = millis();

  t = (now / 100) % 65535;
  //  frequency_factor = fract8(1, abs8(sin8(t)) / 50 + 1);

  t = (now / speed_1) % 65535;
  for (i = 0; i < num_leds; i++) {
    intense = map(sin8(i * frequency_1 * factor + t), 0, 1, 0, 255);
    leds[i] += CHSV(32 + mod1 * 100, 255, lerp8by8(0, intense, intensity));
  }

  t = (now / speed_2) % 65535;
  for (i = 0; i < num_leds; i++) {
    intense = map(sin8(i * frequency_2 * factor + t), 0, 1, 0, 255);
    leds[i] += CHSV(128 + mod2 * 100, 255, lerp8by8(0, intense, intensity));
  }

  // t = (now / speed_3) % 65535;
  // for (i = 0; i < num_leds; i++) {
  //   intense = map(sin8(i * frequency_3 + t), 0, 1, 0, 200);
  //   leds[i] += CHSV(60, 255, lerp8by8(0, intense, intensity));
  // }
}

void render_sine() {
  sine_loop(bar_1, NUM_LEDS_BAR, 1);
  sine_loop(bar_2, NUM_LEDS_BAR, 1);
  sine_loop(matrix, NUM_LEDS_MATRIX, 3);
  sine_loop(strip_1, NUM_LEDS_STRIP, 1);
  sine_loop(strip_2, NUM_LEDS_STRIP, 1);
}
