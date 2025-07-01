#include <FastLED.h>
#include "ColorPalettes.h"
#include "Modes.h"
#include "MatrixLayout.h"
#include <vector>
#include <WiFi.h>
#include <WebServer.h>
#include "Globals.h"
#include "SineLoop.h"
#include "NoisePatterns.h"
#include "LEDHelpers.h"
#include "WaveRenderer.h"

const char* ssid = "ESP32-Access-Point";
const char* password = "12345678";

WebServer server(80);

unsigned long now;

bool long_pulse = true;
unsigned long last_pulse = 0;
const unsigned long short_interval = 1000;
const unsigned long long_interval = 4000;
const unsigned long pulse_rise = 200;
const unsigned long pulse_fall = 1000;
const unsigned long pulse_duration = pulse_rise + pulse_fall;

bool paletteFlipState = false; // false = A, true = B
bool aboveThreshold = false;

void updatePaletteModulated() {
  float threshold = 0.9;

  if (mod1 > threshold && !aboveThreshold) {
    aboveThreshold = true;

    // Flip to the other palette
    paletteFlipState = !paletteFlipState;
    targetPalette = paletteFlipState ? paletteB : paletteA;
  }

  if (mod1 < -threshold && aboveThreshold) {
    aboveThreshold = false;
  }

  // Smooth blend toward the target palette
  nblendPaletteTowardPalette(currentPalette, targetPalette, 1);
}

bool particle_event = false;
unsigned long last_particle_event = 0;
unsigned long event_duration = 0;
unsigned long event_intensity = 0;
unsigned long next_particle_event = 0;

void determine_next_particle_event(long now) {
  next_particle_event = now + random(60, 120) * 1000;
  event_duration = random(5, 20) * 1000;
  event_intensity = random(3, 15);
}

void check_particle_event() {
  now = millis();
  if (!particle_event && now > next_particle_event) {
    particle_event = true;
  } else if (particle_event && now > (next_particle_event + event_duration)) {
    particle_event = false;
    determine_next_particle_event(now);
  }
}

void add_glitter(uint8_t num_particles)
{
  for(int p = 0; p < num_particles; p++) {
    bar_1[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    bar_2[ random16(NUM_LEDS_BAR) ] += CRGB::White;
    strip_1[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
    strip_2[ random16(NUM_LEDS_STRIP) ] += CRGB::White;
  }
}

void next_mode() {
  current_mode = static_cast<Mode>((static_cast<int>(current_mode) + 1) % NUM_MODES);

  switch (current_mode) {
    case noise_orange:
      decay_rate = 255;
      currentPalette = oranje;
      break;
    case noise_blue:
      decay_rate = 255;
      currentPalette = bluePalette;
      break;
    case noise_red:
      decay_rate = 150;
      currentPalette = redPalette;
      break;
    case noise_lava:
      decay_rate = 255;
      currentPalette = LavaColors_p;
      break;
    case sines:
      decay_rate = 255;
      currentPalette = LavaColors_p;
      break;
    case center_pulse:
      decay_rate = 100; // Not really needed since we don't use decay in this mode
      currentPalette = RainbowColors_p;
      break;
    case pulsing:
      decay_rate = 50;
      currentPalette = reddish;
      break;
  }
}

void add_pulse() {
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

void handleModeSwitch() {
  next_mode();
  server.send(200, "text/plain", "switched");
}

void handleSetMode() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();

    // Ensure the value is in the valid range of the enum
    if (val >= 0 && val <= static_cast<int>(center_pulse)) {
      current_mode = static_cast<Mode>(val);
      Serial.println("Mode changed to: " + String(val));
      server.send(200, "text/plain", "Mode changed to " + String(val));
    } else {
      server.send(400, "text/plain", "Invalid mode value");
    }
  } else {
    server.send(400, "text/plain", "Missing value parameter");
  }
}

void handleSetBrightness() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    val = constrain(val, 0, 255);
    currentBrightness = val;
    FastLED.setBrightness(currentBrightness);
    server.send(200, "text/plain", "Set to " + String(val));
  } else {
    server.send(400, "text/plain", "Missing 'value' param");
  }
}


void handleRoot() {
  const char* modeNames[] = {
    "noise_red", "noise_blue", "sines", "noise_lava", "noise_orange", "pulsing", "center_pulse"
  };

  int modeValue = static_cast<int>(current_mode);
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          background-color: #f0f0f0;
          margin: 0;
          padding: 20px;
        }
        h1, h2 {
          font-size: 2em;
          margin-bottom: 20px;
        }
        .slider-container {
          margin-top: 40px;
        }
        input[type=range] {
          width: 80%;
          margin: 20px auto;
        }
        #sliderValue {
          font-size: 1.2em;
        }
        select {
          font-size: 1.2em;
          padding: 10px;
          border-radius: 8px;
        }
      </style>
    </head>
    <body>
      <h1>ESP32 Control</h1>
      <p>Current Mode: <strong>)rawliteral";
  html += modeNames[modeValue];
  html += R"rawliteral(</strong></p>

      <div class="slider-container">
        <p>Set Brightness: <span id="sliderValue">)rawliteral";
  html += currentBrightness;
  html += R"rawliteral(</span></p>
        <input type="range" min="0" max="255" value=")rawliteral";
  html += currentBrightness;
  html += R"rawliteral(" id="valueSlider"
          oninput="updateSliderValue(this.value)"
          onchange="sendValue(this.value)">
      </div>

      <h2>Select Mode</h2>
      <select id="modeSelect" onchange="setMode(this.value))rawliteral";
  html += R"rawliteral(">)rawliteral";

  for (int i = 0; i < sizeof(modeNames) / sizeof(modeNames[0]); ++i) {
    html += "<option value=\"";
    html += i;
    html += "\"";
    if (i == modeValue) html += " selected";
    html += ">";
    html += modeNames[i];
    html += "</option>";
  }

  html += R"rawliteral(
      </select>

      <script>
        function updateSliderValue(val) {
          document.getElementById('sliderValue').innerText = val;
        }
        function sendValue(val) {
          fetch('/setBrightness?value=' + val);
        }
        function setMode(val) {
          fetch('/mode?value=' + val).then(() => location.reload());
        }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void setup() {
  // dev only sanity pause

  Serial.begin(115200);
  pinMode(2, OUTPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/switchMode", handleModeSwitch);
  server.on("/mode", handleSetMode);
  server.on("/setBrightness", handleSetBrightness);

  server.begin();

  determine_next_particle_event(0);

  // LED setup
  FastLED.addLeds<WS2812, BAR1_DATA_PIN, GRB>(bar_1, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, BAR2_DATA_PIN, GRB>(bar_2, NUM_LEDS_BAR).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<WS2812, MATRIX_DATA_PIN, GRB>(matrix, NUM_LEDS_MATRIX).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<WS2801, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN, RGB>(strip_1, NUM_LEDS_STRIP);
  FastLED.addLeds<WS2801, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN, RGB>(strip_2, NUM_LEDS_STRIP);

  FastLED.setBrightness(currentBrightness);

  fill_solid(bar_1, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(bar_2, NUM_LEDS_BAR, CRGB::Black);
  fill_solid(strip_1, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(strip_2, NUM_LEDS_STRIP, CRGB::Black);
  fill_solid(matrix, NUM_LEDS_MATRIX, CRGB::Black);

  FastLED.show();
}

void loop() {
  server.handleClient();

  EVERY_N_MILLISECONDS(1000 * (1 / fps)) {
    updateModifiers();
    updatePaletteModulated();

    fade_all(decay_rate);

    if (current_mode == sines) {
      render_sine();
    } else if (current_mode == center_pulse) {
      renderWaves();
    } else {
      render_noise();
    }

    FastLED.show();
  }

  check_particle_event();
  if (particle_event) {
    add_glitter(event_intensity);
  }


  if (current_mode == pulsing) {
    add_pulse();
  }
}
