#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// === Motor Pins (for MX1508 driver) ===
#define MOTOR_A1 D1 // Left Motor IN1
#define MOTOR_A2 D2 // Left Motor IN2
#define MOTOR_B1 D5 // Right Motor IN1
#define MOTOR_B2 D6 // Right Motor IN2

// === Other Pins ===
const int buzPin = D8;
const int ledPin = D0;
const int wifiLedPin = D4;

// === Constants for Speed Control ===
const int MAX_PWM = 400;          // Maximum PWM value for motors (0-255)
const int MIN_WORKING_SPEED = 80; // Minimum speed where motors actually move
const int DEADZONE = 5;           // Joystick deadzone to prevent drift

// === UDP Setup ===
WiFiUDP udp;
const unsigned int udpPort = 4210;
char udpBuffer[256];

// === Failsafe Timer ===
unsigned long lastUdpCommandTime = 0;
const unsigned long UDP_FAILSAFE_TIMEOUT = 500; // ms

// === WiFi Credentials ===
const char* WIFI_SSID = "wifi-car-1";
const char* WIFI_PASSWORD = "12345678";

void setup() {
  Serial.begin(115200);
  Serial.println("\nWiFi Robot Remote Control Mode (MX1508)");

  pinMode(buzPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(wifiLedPin, OUTPUT);
  
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);

  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);
  stopMotors();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD); // Use variables for SSID and password
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  digitalWrite(wifiLedPin, LOW);

  ArduinoOTA.begin();
  udp.begin(udpPort);
}

void loop() {
  ArduinoOTA.handle();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(udpBuffer, sizeof(udpBuffer) - 1);
    if (len > 0) udpBuffer[len] = 0;

    StaticJsonDocument<192> doc;
    DeserializationError error = deserializeJson(doc, udpBuffer);
    if (!error) {
      int forward = doc["forward"] | 0;
      int backward = doc["backward"] | 0;
      int left = doc["left"] | 0;
      int right = doc["right"] | 0;

      bool lightOn = false;
      bool hornOn = false;
      if (doc["btn"].is<JsonArray>()) {
        for (JsonVariant v : doc["btn"].as<JsonArray>()) {
          String b = v.as<String>();
          if (b == "light") lightOn = true;
          if (b == "horn") hornOn = true;
        }
      }

      int y = forward - backward;
      int x = right - left;

      controlMotion(y, x);

      digitalWrite(ledPin, lightOn ? HIGH : LOW);
      digitalWrite(buzPin, hornOn ? HIGH : LOW);

      // Update failsafe timer
      lastUdpCommandTime = millis();
    }
  }

  // === Failsafe: Stop motors if no UDP command received in timeout ===
  if (millis() - lastUdpCommandTime > UDP_FAILSAFE_TIMEOUT) {
    stopMotors();
    // Optionally, turn off horn and light as well:
    digitalWrite(ledPin, LOW);
    digitalWrite(buzPin, LOW);
  }
}

// === Motor Control Functions (Updated for MX1508) ===

// This function stops the motors by setting both input pins to LOW (coast)
void stopMotors() {
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
}

// Drives the two motors. This function is now optimized for MX1508-style drivers.
// m1 is left motor, m2 is right motor
// Speed is from -255 to +255
void driveMotor(int m1, int m2) {
  m1 = constrain(m1, -255, 255);
  m2 = constrain(m2, -255, 255);

  // --- Control Left Motor (Motor A) ---
  if (m1 > 0) { // Forward
    analogWrite(MOTOR_A1, m1);
    digitalWrite(MOTOR_A2, LOW);
  } else if (m1 < 0) { // Backward
    digitalWrite(MOTOR_A1, LOW);
    analogWrite(MOTOR_A2, -m1);
  } else { // Stop (Coast)
    digitalWrite(MOTOR_A1, LOW);
    digitalWrite(MOTOR_A2, LOW);
  }

  // --- Control Right Motor (Motor B) ---
  if (m2 > 0) { // Forward
    analogWrite(MOTOR_B1, m2);
    digitalWrite(MOTOR_B2, LOW);
  } else if (m2 < 0) { // Backward
    digitalWrite(MOTOR_B1, LOW);
    analogWrite(MOTOR_B2, -m2);
  } else { // Stop (Coast)
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, LOW);
  }
}

// === Improved Joystick-Based Speed Control Logic ===

void controlMotion(int y, int x) {
  // Check deadzone - if joystick is in center, stop motors
  if (abs(y) <= DEADZONE && abs(x) <= DEADZONE) {
    stopMotors();
    Serial.println("Motors stopped - in deadzone");
    return;
  }

  // Calculate speed based on joystick values (0-100)
  float speedY = 0;
  float speedX = 0;

  // Y-axis speed calculation (forward/backward)
  if (abs(y) > DEADZONE) {
    if (y > 0) { // Forward
      // Map joystick value (DEADZONE to 100) to motor speed (MIN_WORKING_SPEED to MAX_PWM)
      speedY = map(y, DEADZONE, 100, MIN_WORKING_SPEED, MAX_PWM);
    } else { // Backward
      // Map joystick value (-DEADZONE to -100) to motor speed (-MIN_WORKING_SPEED to -MAX_PWM)
      speedY = -map(-y, DEADZONE, 100, MIN_WORKING_SPEED, MAX_PWM);
    }
  }

  // X-axis speed calculation (left/right turning)
  if (abs(x) > DEADZONE) {
    if (x > 0) { // Right turn
      speedX = map(x, DEADZONE, 100, MIN_WORKING_SPEED, MAX_PWM);
    } else { // Left turn
      speedX = -map(-x, DEADZONE, 100, MIN_WORKING_SPEED, MAX_PWM);
    }
  }

  // Calculate individual motor speeds for tank steering
  float leftMotorSpeed = speedY - speedX;   // Left motor: forward/back minus turning
  float rightMotorSpeed = speedY + speedX;  // Right motor: forward/back plus turning

  // Normalize speeds if they exceed maximum
  float maxMagnitude = max(abs(leftMotorSpeed), abs(rightMotorSpeed));
  if (maxMagnitude > MAX_PWM) {
    float scaleFactor = (float)MAX_PWM / maxMagnitude;
    leftMotorSpeed *= scaleFactor;
    rightMotorSpeed *= scaleFactor;
  }

  // Drive the motors
  driveMotor((int)leftMotorSpeed, (int)rightMotorSpeed);

  // Debug output
  Serial.print("Joystick Y: "); Serial.print(y);
  Serial.print(" | X: "); Serial.print(x);
  Serial.print(" | SpeedY: "); Serial.print(speedY);
  Serial.print(" | SpeedX: "); Serial.print(speedX);
  Serial.print(" | Left Motor: "); Serial.print((int)leftMotorSpeed);
  Serial.print(" | Right Motor: "); Serial.println((int)rightMotorSpeed);
}