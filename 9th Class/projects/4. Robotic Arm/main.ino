/*
 * ESP8266 Robotic Arm with Aarti Mode
 * 
 * Features:
 * - WiFi Access Point + Webserver
 * - Control 4 servos manually with sliders
 * - Special "Aarti Mode" button -> base + shoulder move in circle
 * - Stop button to cancel aarti
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <math.h>

// WiFi credentials
const char* ssid = "Robotic_Arm_AP";
const char* password = "12345678";

// Web server
ESP8266WebServer server(80);

// Servo objects
Servo servo1; // Base
Servo servo2; // Shoulder
Servo servo3; // Elbow
Servo servo4; // Gripper

// Pins (for Wemos D1 Mini / NodeMCU)
#define SERVO_PIN_1 D1 
#define SERVO_PIN_2 D2
#define SERVO_PIN_3 D5
#define SERVO_PIN_4 D6

// Aarti mode flag
bool aartiMode = false;

// HTML Page with Sliders + Aarti Buttons
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Robotic Arm Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body { font-family: Arial; text-align: center; background: #f0f0f0; }
    .container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 12px; }
    h1 { color: #005a9c; }
    .servo-control { margin: 20px 0; }
    input[type=range] { width: 90%; }
    .btn {
      display: inline-block; padding: 12px 20px; margin: 10px;
      background: #007bff; color: #fff; border: none; border-radius: 8px;
      font-size: 1em; cursor: pointer;
    }
    .btn.stop { background: #dc3545; }
  </style>
</head>
<body>
<div class="container">
  <h1>Robotic Arm Control</h1>

  <div class="servo-control">
    <label>Base Servo</label>
    <input type="range" id="servo1" min="0" max="180" value="90" oninput="sendValue(this)">
    <div>Angle: <span id="value1">90</span>°</div>
  </div>

  <div class="servo-control">
    <label>Shoulder Servo</label>
    <input type="range" id="servo2" min="0" max="180" value="90" oninput="sendValue(this)">
    <div>Angle: <span id="value2">90</span>°</div>
  </div>

  <div class="servo-control">
    <label>Elbow Servo</label>
    <input type="range" id="servo3" min="0" max="180" value="90" oninput="sendValue(this)">
    <div>Angle: <span id="value3">90</span>°</div>
  </div>

  <div class="servo-control">
    <label>Gripper Servo</label>
    <input type="range" id="servo4" min="0" max="180" value="90" oninput="sendValue(this)">
    <div>Angle: <span id="value4">90</span>°</div>
  </div>

  <h2>Aarti Mode</h2>
  <button class="btn" onclick="startAarti()">Start Aarti</button>
  <button class="btn stop" onclick="stopAarti()">Stop Aarti</button>
</div>

<script>
function sendValue(slider) {
  var id = slider.id;
  var value = slider.value;
  document.getElementById("value" + id.slice(-1)).innerText = value;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/control?id=" + id + "&value=" + value, true);
  xhr.send();
}

function startAarti() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/aarti?mode=start", true);
  xhr.send();
}

function stopAarti() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/aarti?mode=stop", true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// ================= Functions =================

// Serve main page
void handleRoot() {
  server.send(200, "text/html", webpage);
}

// Manual control
void handleControl() {
  if (server.hasArg("id") && server.hasArg("value")) {
    String idStr = server.arg("id");
    int value = server.arg("value").toInt();

    if (idStr == "servo1") servo1.write(value);
    else if (idStr == "servo2") servo2.write(value);
    else if (idStr == "servo3") servo3.write(value);
    else if (idStr == "servo4") servo4.write(value);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// Aarti Mode Handler
void handleAarti() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "start") {
      aartiMode = true;
      server.send(200, "text/plain", "Aarti Started");
    } else if (mode == "stop") {
      aartiMode = false;
      server.send(200, "text/plain", "Aarti Stopped");
      servo1.write(90);
      servo2.write(90);
    }
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// Perform Aarti Motion (SG90 style)
// Perform Aarti Motion (Base + Shoulder + Elbow = Realistic)
// Perform Aarti Motion (All Servos for Realistic Effect)
void performAartiStep() {
  static int angle = 0;
  angle = (angle + 4) % 360;  // slow smooth circle

  // Base (left-right): wide motion
  int base = 90 + 70 * sin(angle * 3.1416 / 180.0);     

  // Shoulder (up-down): big arc
  int shoulder = 90 + 60 * cos(angle * 3.1416 / 180.0); 

  // Elbow: smaller counter movement (adds natural curve)
  int elbow = 90 + 25 * sin((angle + 120) * 3.1416 / 180.0);  



  // Clamp values for safety
  base = constrain(base, 0, 180);
  shoulder = constrain(shoulder, 0, 180);
  elbow = constrain(elbow, 0, 180);


  // Move servos
  servo1.write(base);
  servo2.write(shoulder);
  servo3.write(elbow);
 

  delay(40); // smoother & realistic speed
}



// ================= Setup =================
void setup() {
  Serial.begin(115200);

  // Attach servos
  servo1.attach(SERVO_PIN_1);
  servo2.attach(SERVO_PIN_2);
  servo3.attach(SERVO_PIN_3);
  servo4.attach(SERVO_PIN_4);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);

  // WiFi AP mode
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Server routes
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/aarti", handleAarti);

  server.begin();
  Serial.println("HTTP server started");
}

// ================= Loop =================
void loop() {
  server.handleClient();

  if (aartiMode) {
    performAartiStep();
  }
}
