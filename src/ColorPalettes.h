#pragma once 
#include "FastLED.h"

constexpr int NUM_PALETTES = 5;
extern const CRGBPalette16 palettes[NUM_PALETTES];
extern String paletteNames[NUM_PALETTES];
extern int paletteIndex;

const CRGBPalette16 redPalette(
  CHSV(0, 255, 255),
  CHSV(0, 255, 100),
  CHSV(160, 255, 0),
  CHSV(240, 255, 255));

const CRGBPalette16 bluePalette(
  CHSV(160, 255, 255),
  CHSV(160, 255, 100),
  CHSV(160, 255, 0),
  CHSV(240, 255, 255));

const CRGBPalette16 oranje(
  CHSV(20, 255, 255),
  CHSV(20, 255, 255),
  CHSV(206, 255, 0),
  CHSV(206, 255, 255));


void setPalette(int paletteIndex);
void setPalette(CRGBPalette16 palette);
void nextPalette();
void updatePalettes();