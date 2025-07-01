#include "Modes.h"
#include "Globals.h"
#include "ColorPalettes.h"

void next_mode() {
  current_mode = static_cast<Mode>((static_cast<int>(current_mode) + 1) % NUM_MODES);

  switch (current_mode) {
    case noise_orange:
      decay_rate = 255;
      currentPalette = oranje;
      break;
    case noise_blue:
      decay_rate = 255;
      currentPalette = bluePalette;
      break;
    case noise_red:
      decay_rate = 150;
      currentPalette = redPalette;
      break;
    case noise_lava:
      decay_rate = 255;
      currentPalette = LavaColors_p;
      break;
    case sines:
      decay_rate = 255;
      currentPalette = LavaColors_p;
      break;
    case center_pulse:
      decay_rate = 100; // Not really needed since we don't use decay in this mode
      currentPalette = RainbowColors_p;
      break;
    case pulsing:
      decay_rate = 50;
      currentPalette = reddish;
      break;
  }
}
