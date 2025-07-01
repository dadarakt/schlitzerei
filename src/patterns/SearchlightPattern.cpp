  // SearchlightPattern.cpp
  #include "SearchlightPattern.h"
  #include "Globals.h"
  #include "LEDHelpers.h"
  
  extern double mod1;
  extern double mod2;
  
  bool searchlight_active = false;
  unsigned long searchlight_start = 0;
  unsigned long searchlight_duration = 1500;
  
  void initSearchlight() {
    searchlight_active = false;
    searchlight_start = 0;
  }
  
  void updateSearchlight() {
    static unsigned long next_activation_time = 0;
    static unsigned long active_duration = 0;
  
    unsigned long now = millis();
  
    if (searchlight_active) {
      if (now - searchlight_start > active_duration) {
        searchlight_active = false;
        next_activation_time = now + random(10000, 20000); // 10–20 sec off
      }
    } else {
      if (now >= next_activation_time) {
        searchlight_active = true;
        searchlight_start = now;
        active_duration = random(10000, 20000); // 10–20 sec on
      }
    }
  }
  
  void renderRedGlimmer(CRGB* leds, int num_leds, uint16_t offset = 0) {
    float speed = 25 + ((mod1 + 1.0f) / 2.0f) * 4.0f; // modulate pulsing speed
    float pulse = ((sin8(millis() / speed) - 128) / 128.0f + 1.0f) * 1.1f; // stronger pulsing // heartbeat-like
    for (int i = 0; i < num_leds; i++) {
      uint8_t noise = inoise8(i * 10, millis() / 8 + offset);
      int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) / 2.0f) * 80.0f;
      uint8_t red = min(red_intensity, 180);
      leds[i] = CRGB(red, red / 10, red / 20);  // full red range, warm but no white  // warm metal tone  // strong clamp to avoid whiteout
    }
  }
  
  void renderRedGlimmerMatrix() {
    float speed = 25 + ((mod1 + 1.0f) / 2.0f) * 8.0f; // modulate pulsing speed
    float pulse = (sin8(millis() / speed) - 128) / 128.0f + 1.0f;
    for (uint8_t x = 0; x < COLS; x++) {
      for (uint8_t y = 0; y < ROWS; y++) {
        uint8_t noise = inoise8(x * 15, y * 15, millis() / 10);
        int red_intensity = 30 + noise * 0.4f + ((mod1 + 1.0f) / 2.0f) * 80.0f;
        uint8_t red = min(red_intensity, 180);
        matrix[XY(x, y)] = CRGB(red, red / 10, red / 20);  // full red range, warm but no white  // warm metal tone  // strong clamp to avoid whiteout
      }
    }
  }
  
  void renderSearchlightCombined() {
    extern CRGB strip_1[];
    extern CRGB strip_2[];
    const int NUM_LEDS_STRIP = NUM_LEDS_BAR;
    if (!searchlight_active) return;
  
    float total_width = NUM_LEDS_BAR * 2 + COLS + NUM_LEDS_STRIP * 2;
    float oscillation = sinf(millis() / 1000.0f) * 0.5f + 0.5f; // oscillate 0.0 to 1.0
    float pos = oscillation * total_width;
    float width = 6 + ((mod2 + 1.0f) / 2.0f) * 10.0f; // softer edges modulated
  
    // Strip 1
    for (int i = 0; i < NUM_LEDS_STRIP; i++) {
      float global_pos = NUM_LEDS_STRIP - 1 - i;
      float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
      CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
      strip_1[i] += white;
    }
  
    // Bar 1
    for (int i = 0; i < NUM_LEDS_BAR; i++) {
      float global_pos = NUM_LEDS_STRIP + NUM_LEDS_BAR - 1 - i;
      float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
      CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
      bar_1[i] += white;
    }
  
    // Matrix
    for (uint8_t x = 0; x < COLS; x++) {
      float global_pos = NUM_LEDS_STRIP + NUM_LEDS_BAR + x;
      float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
      CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
      for (uint8_t y = 0; y < ROWS; y++) {
        matrix[XY(x, y)] += white;
      }
    }
  
    // Bar 2
    for (int i = 0; i < NUM_LEDS_BAR; i++) {
      float global_pos = NUM_LEDS_STRIP + NUM_LEDS_BAR + COLS + i;
      float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
      CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
      bar_2[i] += white;
    }
  
    // Strip 2
    for (int i = 0; i < NUM_LEDS_STRIP; i++) {
      float global_pos = NUM_LEDS_STRIP + NUM_LEDS_BAR + COLS + NUM_LEDS_BAR + i;
      float brightness = max(0.0f, 1.0f - abs(global_pos - pos) / width);
      CRGB white = CRGB(brightness * 255, brightness * 255, brightness * 255);
      strip_2[i] += white;
    }
  }
  
  void renderSearchlightPattern() {
  extern CRGB strip_1[];
  extern CRGB strip_2[];
  const int NUM_LEDS_STRIP = NUM_LEDS_BAR;

  renderRedGlimmer(bar_1, NUM_LEDS_BAR);
  renderRedGlimmer(bar_2, NUM_LEDS_BAR, 1000);
  renderRedGlimmer(strip_1, NUM_LEDS_STRIP, 2000);
  renderRedGlimmer(strip_2, NUM_LEDS_STRIP, 3000);
  renderRedGlimmerMatrix();
  renderSearchlightCombined();
}
