#pragma once 
#include "FastLED.h"

const CRGB neutral_white = CHSV(50, 150, 255);

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

const CRGBPalette16 reddish(
  CHSV(250, 255, 30),
  CHSV(250, 255, 50),
  CHSV(10, 255, 30),
  CHSV(10, 255, 50));

extern const CRGBPalette16 palettes[];

void setColorPalette(CRGBPalette16 palette);
void nextPalette();
void updatePalettes();