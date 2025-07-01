#include "Server.h"
#include "Globals.h"
#include "Modes.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "BlindingLights";
const char* password = "lichtung";
WebServer server(80);

void updateServer() {
  server.handleClient();
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

void initServer() {
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
}