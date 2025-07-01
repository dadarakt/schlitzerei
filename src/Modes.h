// Modes.h
#ifndef MODES_H
#define MODES_H

void next_mode();

enum Mode {
  noise_red,
  noise_blue,
  sines,
  noise_lava,
  noise_orange,
  pulsing,
  center_pulse,
  NUM_MODES
};

#endif