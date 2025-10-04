#include "MuxInput.h"
#include "Globals.h"
#include "patterns/Patterns.h"
#include "ColorPalettes.h"
#include "StrobeEffect.h"

// edge detection flags
bool lastBtn0 = false;
bool lastBtn1 = false;
bool lastBtn2 = false;
bool lastBtn3 = false;

// ---- storage
bool gButtons[4] = {false, false, false, false};
int  gPotRaw = 0;

using namespace MuxCfg;

static inline void selectChannel(uint8_t ch) {
  digitalWrite(S0, ch & 0x01);
  digitalWrite(S1, (ch >> 1) & 0x01);
  digitalWrite(S2, (ch >> 2) & 0x01);
  delayMicroseconds(10); // mux settle
}

static inline int readMux(uint8_t ch) {
  selectChannel(ch);
  (void)analogRead(MUX_SIG);     // dummy read to charge S/H cap
  delayMicroseconds(40);
  return analogRead(MUX_SIG);
}

void muxInit() {
  // Address pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  // ADC setup for full ~0..3.3V span on ADC1
  analogReadResolution(12);                  // 0..4095
  analogSetPinAttenuation(MUX_SIG, ADC_11db);
}

void muxRead() {
  // Pot → brightness (re-apply to FastLED)
  gPotRaw = readMux(POT_CH);
  currentBrightness = map(gPotRaw, 0, 4095, 255, 0);
  FastLED.setBrightness(currentBrightness);   // <-- needed every update

  // Buttons (pull-down wiring => LOW = pressed)
  for (int i = 0; i < 4; i++) {
    const int v = readMux(BTN_CH[i]);
    gButtons[i] = (v < PRESS_THRESHOLD);
  }

  bool b0 = gButtons[0];
  if (b0 && !lastBtn0) {
    nextPattern();
  }
  lastBtn0 = b0;

  bool b1 = gButtons[1];
  if (b1 && !lastBtn1) {
    nextPalette();
  }
  lastBtn1 = b1;

  bool b2 = gButtons[2]; 
  if (b2 && !lastBtn2) {
    //nextPattern();
    //nextPalette();
  }
  lastBtn2 = b2;

  bool b3 = gButtons[3];                 // true when pressed
  if (b3 && !lastBtn3) {
    startStrobeContinuous(true);         // choose white; pass false to use palette
  } else if (!b3 && lastBtn3) {
    stopStrobe();
  }
  lastBtn3 = b3;
}