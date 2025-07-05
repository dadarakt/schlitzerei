#include "Server.h"
#include "Globals.h"
#include "patterns/Patterns.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ColorPalettes.h"
#include "FastLED.h"

const char* ssid = "BlindingLights";
const char* password = "lichtung";
WebServer server(80);

void updateServer() {
  server.handleClient();
}

void handlePatternSwitch() {
  nextPattern();
  server.send(200, "text/plain", "switched");
}

void handleSetPalette() {
    if (server.hasArg("value")) {
        int val = server.arg("value").toInt();

        if (val >= 0 && val < NUM_PALETTES) {
            setPalette(val);
            server.send(200, "text/plain", "Palette changed to " + paletteNames[val]);
        }
    } else {
        server.send(400, "text/plain", "Missing value parameter");
    }
}

void handleSetPattern() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();

    // Ensure the value is in the valid range of the enum
    if (val >= 0 && val < static_cast<int>(NUM_PATTERNS)) {
      Pattern nextPattern = static_cast<Pattern>(val);
      setPattern(nextPattern);
      server.send(200, "text/plain", "Pattern changed to " + String(val));
    } else {
      server.send(400, "text/plain", "Invalid pattern value");
    }
  } else {
    server.send(400, "text/plain", "Missing value parameter");
  }
}

void handleToggleAutoCyclePattern() {
  if (server.hasArg("value")) {
    autoCyclePatterns = server.arg("value") == "1";
    server.send(200, "text/plain", "autoCyclePatterns: " + String(autoCyclePatterns));
  } else {
    server.send(400, "text/plain", "Missing value parameter");
  }
}

void handleToggleAutoCyclePalette() {
  if (server.hasArg("value")) {
    autoCyclePalettes = server.arg("value") == "1";
    server.send(200, "text/plain", "autoCyclePalettes: " + String(autoCyclePalettes));
  } else {
    server.send(400, "text/plain", "Missing value parameter");
  }
}

void handleToggleStrobeActive() {
  if (server.hasArg("value")) {
    strobeActive = server.arg("value") == "1";
    server.send(200, "text/plain", "strobeActive: " + String(autoCyclePalettes));
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
  const char* patternNames[] = {"Sines", "Noise", "Waves", "Search Light"};

  int patternValue = static_cast<int>(currentPattern);
  int paletteValue = paletteIndex;

  // header & style
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
    </head>)rawliteral";

  // Brightness control
  html += R"rawliteral(
    <body>
      <h1>LIGHTS</h1>
      <div class="slider-container">
        <h2>Brightness: <span id="sliderValue">
        </span></h2>
        <input type="range" min="0" max="255" value=")rawliteral";
  html += currentBrightness;
  html += R"rawliteral(" id="valueSlider"
          oninput="updateSliderValue(this.value)"
          onchange="sendValue(this.value)">
      </div>)rawliteral";

  // Pattern setting
  html += R"rawliteral(
    <h2>Select Pattern</h2>
    <p>
        <label>
        <input type="checkbox" id="cyclePattern" onchange="toggleCyclePattern(this.checked)">
        Auto Cycle Pattern
        </label>
    </p>
    <select id="patternSelect" onchange="setPattern(this.value)">)rawliteral";
  for (int i = 0; i < sizeof(patternNames) / sizeof(patternNames[0]); ++i) {
    html += "<option value=\"";
    html += i;
    html += "\"";
    if (i == patternValue) html += " selected";
    html += ">";
    html += patternNames[i];
    html += "</option>";
  }
  html += R"rawliteral(</select>)rawliteral";

  // Color setting
  html += R"rawliteral(
    <h2>Select Colors</h2>
    <p>
        <label>
        <input type="checkbox" id="cyclePalette" onchange="toggleCyclePalette(this.checked)">
        Auto Cycle Palette
        </label>
    </p>
    <select id="paletteSelect" onchange="setPalette(this.value)">)rawliteral";

  for (int i = 0; i < NUM_PALETTES; ++i) {
    html += "<option value=\"";
    html += i;
    html += "\"";
    if (i == paletteValue) html += " selected";
    html += ">";
    html += paletteNames[i];
    html += "</option>";
  }
  html += R"rawliteral(</select>)rawliteral";

  html += R"rawliteral("
    <p>
        <label>
        <input type="checkbox" id="strobeActive" onchange="toggleStrobeActive(this.checked)">
        Strobe Active
        </label>
    </p>
  )rawliteral";

  // JS
  html += R"rawliteral(
  <script>
    window.onload = function() {
      document.getElementById('cyclePattern').checked = )rawliteral";
  html += (autoCyclePatterns ? "true" : "false");
  html += R"rawliteral(;
        document.getElementById('cyclePalette').checked = )rawliteral";
  html += (autoCyclePalettes ? "true" : "false");
  html += R"rawliteral(;
        document.getElementById('strobeActive').checked = )rawliteral";
  html += (strobeActive ? "true" : "false");
  html += R"rawliteral(;
    };
    function toggleCyclePattern(isChecked) {
      fetch('/toggleAutoCyclePattern?value=' + (isChecked ? '1' : '0'));
    }
    function toggleCyclePalette(isChecked) {
      fetch('/toggleAutoCyclePalette?value=' + (isChecked ? '1' : '0'));
    }
    function toggleStrobeActive(isChecked) {
      fetch('/toggleStrobeActive?value=' + (isChecked ? '1' : '0'));
    }
    function updateSliderValue(val) {
      document.getElementById('sliderValue').innerText = val;
    }
    function sendValue(val) {
      fetch('/setBrightness?value=' + val);
    }
    function setPattern(val) {
      fetch('/pattern?value=' + val).then(() => location.reload());
    }
    function setPalette(val) {
      fetch('/palette?value=' + val).then(() => location.reload());
    }
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void initServer() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  // Routes
  server.on("/", handleRoot);
  server.on("/switchPattern", handlePatternSwitch);
  server.on("/palette", handleSetPalette);
  server.on("/pattern", handleSetPattern);
  server.on("/setBrightness", handleSetBrightness);
  server.on("/toggleAutoCyclePattern", handleToggleAutoCyclePattern);
  server.on("/toggleAutoCyclePalette", handleToggleAutoCyclePalette);
  server.on("/toggleStrobeActive", handleToggleStrobeActive);

  server.begin();
}