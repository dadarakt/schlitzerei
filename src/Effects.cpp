#include <FastLED.h>
#include "Globals.h"
#include "LEDHelpers.h"

using namespace LEDStore;

const unsigned long short_interval = 1000;
const unsigned long long_interval = 4000;
const unsigned long pulse_rise = 200;
const unsigned long pulse_fall = 1000;

const unsigned long pulse_duration = pulse_rise + pulse_fall;
static bool long_pulse = true;
static unsigned long last_pulse = 0;

static bool particle_event = false;
static unsigned long last_particle_event = 0;
static unsigned long event_duration = 0;
static unsigned long event_intensity = 0;
static unsigned long next_particle_event = 0;
long now = 0;

void determineNextParticleEffect(long now) {
  next_particle_event = now + random(60, 120) * 1000;
  event_duration = random(5, 20) * 1000;
  event_intensity = random(3, 15);
}

void updateParticleEffect() {
  if (!particleEffectActive) return;

  now = millis();
  if (!particle_event && now > next_particle_event) {
    particle_event = true;
  } else if (particle_event && now > (next_particle_event + event_duration)) {
    particle_event = false;
    determineNextParticleEffect(now);
  }
}

void addGlitter(uint8_t num_particles)
{
  if (!glitterEffectActive) return;

  for(int p = 0; p < num_particles; p++) {
    bar_1[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    bar_2[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    strip_1[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
    strip_2[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
  }
}

void updatePulse() {
  if (!pulseEffectActive) return;

  now = millis();
  if (long_pulse && now > (last_pulse + long_interval)) {
    last_pulse = now;
    long_pulse = !long_pulse;
    return;
  }

  if (!long_pulse && now > (last_pulse + short_interval)) {
    last_pulse = now;
    long_pulse = !long_pulse;
    return;
  }

  if (now < (last_pulse + pulse_duration)) {
    unsigned long t_pulse = now - last_pulse;

    if (t_pulse <= pulse_rise) {
       float amount = t_pulse / (float)pulse_rise;
       add_solid_all(CRGB(amount * 255, amount * 50, amount * 50));
       return;
    }
  }
}

void updateEffects() {
  updateParticleEffect();
  addGlitter(1);
  updatePulse();
}