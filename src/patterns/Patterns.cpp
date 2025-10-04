#include "Patterns.h"
#include "Globals.h"
#include "ColorPalettes.h"

Pattern currentPattern = dropPattern;

void setPattern(Pattern pattern) {
  currentPattern = pattern;
  switch (pattern) {
    case dropPattern:
      decay_rate = 5;
      break;
    case sinePattern:
      decay_rate = 255;
      break;
    case noisePattern: 
      decay_rate = 50;
      break;
    case wavePattern:
      decay_rate = 100; // Not really needed since we don't use decay in this mode
      break;
    case searchlightPattern:
      decay_rate = 100;
      break;
    default: break;
  }
}

void nextPattern() {
  Pattern nextPattern = static_cast<Pattern>((static_cast<int>(currentPattern) + 1) % NUM_PATTERNS);
  setPattern(nextPattern);
}

