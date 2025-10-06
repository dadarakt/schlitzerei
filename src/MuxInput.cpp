#include "MuxInput.h"
#include "Globals.h"
#include "patterns/Patterns.h"
#include "ColorPalettes.h"
#include "StrobeEffect.h"
#include "CrowdBlinder.h"

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
  currentBrightness = map(gPotRaw, 0, 4095, 255, 30);
  FastLED.setBrightness(currentBrightness);

  // Buttons (pull-down wiring => LOW = pressed)
  for (int i = 0; i < 4; i++) {
    const int v = readMux(BTN_CH[i]);
    gButtons[i] = (v < PRESS_THRESHOLD);
  }

  // -------- Buttons 0 & 1: trigger on RELEASE --------
  bool b0 = gButtons[0];
  bool b1 = gButtons[1];

  // Detect if both are pressed simultaneously
  static bool comboActive = false;

  // Combo press: both buttons held down
  if (b0 && b1) {
    comboActive = true;
  }

  // Button 0 released
  if (!b0 && lastBtn0) {
    if (comboActive && !b1) {
      // both were pressed together, now both released
      // → trigger combo action
      // (example) switch to a special “PanelPulse” pattern:
      autoCyclePalettes = !autoCyclePalettes;
      autoCyclePatterns = !autoCyclePatterns;
      comboActive = false;
    } else if (!comboActive) {
      // single button 0 release
      nextPattern();
    }
  }

  // Button 1 released
  if (!b1 && lastBtn1) {
    if (!comboActive) {
      // single button 1 release
      nextPalette();
    }
    // if comboActive, combo already handled above
  }

  lastBtn0 = b0;
  lastBtn1 = b1;

  // -------- Button 2: CrowdBlinder (while held) --------
  bool b2 = gButtons[2];
  if (b2 != lastBtn2) {
    crowdBlinderEnable(b2);
    lastBtn2 = b2;
  }

  // -------- Button 3: Strobe (while held) --------
  bool b3 = gButtons[3];
  if (b3 && !lastBtn3) {
    startStrobeContinuous(true);
  } else if (!b3 && lastBtn3) {
    stopStrobe();
  }
  lastBtn3 = b3;
}
