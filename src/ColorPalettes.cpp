#include "ColorPalettes.h"
#include "FastLED.h"
#include "Globals.h"

int paletteIndex = 0;

const CRGBPalette16 palettes[NUM_PALETTES] = {
    oranje,
    redPalette,
    ForestColors_p,
    CloudColors_p,
    PinkBluePalette_p,
    NeonContrastPalette_p
};

String paletteNames[NUM_PALETTES] = { 
    "Oranje", "Reds", "Forest", "Cloud", "Pink-Blue", "Neon"
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

void setPalette(int index) {
    paletteIndex = index;
    targetPalette = palettes[paletteIndex];
}

void setPalette(CRGBPalette16 palette) {
    targetPalette = palette;
    paletteIndex = findPaletteIndex(palette);
}

void nextPalette() {
    int nextPaletteIndex = (paletteIndex + 1) % NUM_PALETTES;
    setPalette(nextPaletteIndex);
}

void updatePalettes() {
  nblendPaletteTowardPalette(currentPalette, targetPalette, 5);
}
