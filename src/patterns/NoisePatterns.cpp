#include "NoisePatterns.h"
#include "FastLED.h"
#include "Globals.h"
#include "LEDHelpers.h"

uint8_t data, old_data, new_data;
uint16_t i_offset, j_offset;

uint16_t minSpeed = 5;
uint16_t maxSpeed = 20;
uint16_t speed = 5;  // a nice starting speed, mixes well with a scale of 100
uint16_t minScale = 1;
uint16_t maxScale = 50;
uint16_t scale = 10;  // scale 50 is nice marbly, lower gets to shifting colors, higher to flimmering

int x = random16();
int y = random16();
int z = random16();

uint8_t data_smoothing = speed < 50
                           ? 200 - (speed * 4)
                           : 0;

// This is the array that we keep our computed noise values in
uint16_t noise[MATRIX_MAX_DIMENSION][MATRIX_MAX_DIMENSION];

static uint8_t led_index;

void fill_noise_8() {
  int i,j;

  for (i = 0; i < MATRIX_MAX_DIMENSION; i++) {
    i_offset = scale * i;
    for (j = 0; j < MATRIX_MAX_DIMENSION; j++) {
      j_offset = scale * j;

      data = inoise8(x + i_offset, y + j_offset, z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (data_smoothing) {
        old_data = noise[i][j];
        new_data = scale8(old_data, data_smoothing) + scale8(data, 256 - data_smoothing);
        data = new_data;
      }

      noise[i][j] = data;
    }
  }

  z += speed / 10;

  // apply slow drift to X and Y, just for visual variation.
  x += speed / 16;
  y -= speed / 32;
}

void map_noise_to_leds_with_palette(CRGB leds[], uint8_t rows, uint8_t cols, uint8_t offset) {
  static uint8_t colorLoop = 1;
  static uint8_t ihue = 0;

  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[(j+offset) % MATRIX_MAX_DIMENSION][(i+offset) % MATRIX_MAX_DIMENSION];
      uint8_t bri = noise[(i+offset) % MATRIX_MAX_DIMENSION][(j+offset) % MATRIX_MAX_DIMENSION];

      // if this palette is a 'loop', add a slowly-changing base value
      if (colorLoop) {
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if (bri > 127) {
        bri = 255;
      } else {
        bri = dim8_raw(bri * 2);
      }

      CRGB color = ColorFromPalette(currentPalette, index, bri);
      leds[XY(i, j)] += color;
    }
  }

  ihue += 1;
}

void map_noise_to_leds_with_hue() {
  static uint8_t ihue = 0;

  for (int i = 0; i < COLS; i++) {
    for (int j = 0; j < ROWS; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
      led_index = XY(i, j);
      matrix[led_index] = CRGB::Black;

      //leds[XY(i,j)] = CHSV(noise[j][i], 255, noise[i][j]);
      // constrained hue for more uniform colors
      matrix[XY(i, j)] += CHSV(ihue + (noise[j][i] >> 2), 255, noise[i][j]);
    }
  }
  ihue += 1;
}

void render_noise() {
  speed = minSpeed + mod2 * (maxSpeed - minSpeed);
  scale = minScale + mod1 * (maxScale - minScale);
  fill_noise_8();
  //map_noise_to_leds_with_hue();
  map_noise_to_leds_with_palette(matrix, ROWS, COLS, 0);
  map_noise_to_leds_with_palette(bar_1, NUM_LEDS_BAR, 1, 50);
  map_noise_to_leds_with_palette(bar_2, NUM_LEDS_BAR, 1, 100);
  map_noise_to_leds_with_palette(strip_1, NUM_LEDS_STRIP, 1, 50);
  map_noise_to_leds_with_palette(strip_2, NUM_LEDS_STRIP, 1, 100);
}


