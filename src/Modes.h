// Modes.h
#ifndef MODES_H
#define MODES_H

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

void setMode(Mode mode);
void nextMode();

#endif