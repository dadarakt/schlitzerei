#include "ColorPalettes.h"
#include "FastLED.h"
#include "Globals.h"

int paletteIndex = 0;

const int NUM_PALETTES = 6;

const CRGBPalette16 palettes[NUM_PALETTES] = {
    reddish,
    oranje,
    bluePalette,
    redPalette,
    ForestColors_p,
    CloudColors_p
};

#include <iostream>
int findPaletteIndex(CRGBPalette16 palette)
{
    for (int i = 0; i < NUM_PALETTES; ++i)
    { 
        if (palettes[i] == palette) return i;
    }
    return 0;
}

void setPalette(CRGBPalette16 palette) {
    targetPalette = palette;
    paletteIndex = findPaletteIndex(palette);
}

void nextPalette() {
    paletteIndex = (paletteIndex + 1) % NUM_PALETTES;
    targetPalette = palettes[paletteIndex];
}

void updatePalettes() {
  nblendPaletteTowardPalette(currentPalette, targetPalette, 1);
}
