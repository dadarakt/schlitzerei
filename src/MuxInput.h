#pragma once
#include <Arduino.h>

// --- Public data (updated after calling muxRead())
extern bool gButtons[4];  // pressed = true
extern int  gPotRaw;      // 0..4095

// --- Config (adjust to your wiring)
namespace MuxCfg {
  // MUX pins
  constexpr int MUX_SIG = 35;    // ADC1 (GPIO35)
  constexpr int S0 = 25;
  constexpr int S1 = 26;
  constexpr int S2 = 27;

  // Channels
  constexpr uint8_t BTN_CH[4] = {2, 1, 0, 3};
  constexpr uint8_t POT_CH    = 4;

  // Thresholds (pull-up buttons: LOW when pressed)
  constexpr int PRESS_THRESHOLD = 1000;  // pressed if < ~1000
}

// --- API
void muxInit();
void muxRead();
