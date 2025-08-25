// Motor Test for ESP32 DevKit V1 with L298N
// Pins: IN1=15, IN2=2, IN3=4, IN4=16

#define MOTOR_IN1 15 // D15 / GPIO15 (Right motor forward)
#define MOTOR_IN2 2  // D2  / GPIO2  (Right motor backward)
#define MOTOR_IN3 4  // D4  / GPIO4  (Left motor forward)
#define MOTOR_IN4 16 // RX2 / GPIO16 (Left motor backward)

#define TEST_DELAY 1000 // ms

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  stopMotors();
  Serial.println("Motor Test Starting...");
  delay(2000);
}

void loop() {
  Serial.println("Right Motor Forward");
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  delay(TEST_DELAY);
  stopMotors();
  delay(500);

  Serial.println("Right Motor Backward");
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  delay(TEST_DELAY);
  stopMotors();
  delay(500);

  Serial.println("Left Motor Forward");
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  delay(TEST_DELAY);
  stopMotors();
  delay(500);

  Serial.println("Left Motor Backward");
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
  delay(TEST_DELAY);
  stopMotors();
  delay(1000);
}

void stopMotors() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}
