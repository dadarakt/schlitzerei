#pragma once
#include <FastLED.h>

void updateEffects();
void determineNextParticleEffect(long now);
void updateParticleEffect();
void addGlitter(uint8_t num_particles);
void updatePulse();