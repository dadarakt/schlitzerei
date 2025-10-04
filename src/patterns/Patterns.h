// Modes.h
#pragma once

enum Pattern {
  dropPattern,
  panelPulsePattern,
  sinePattern,
  noisePattern,
  wavePattern,
  searchlightPattern,
  NUM_PATTERNS
};

extern Pattern currentPattern;

void setPattern(Pattern pattern);
void nextPattern();