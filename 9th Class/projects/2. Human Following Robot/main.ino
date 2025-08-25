// Human Following Robot - ESP32

// Sensors
#define LEFT_TRIG 14
#define LEFT_ECHO 27
#define CENTER_TRIG 26
#define CENTER_ECHO 25
#define RIGHT_TRIG 33
#define RIGHT_ECHO 32

// Motors
#define MOTOR_IN1 15
#define MOTOR_IN2 2
#define MOTOR_IN3 4
#define MOTOR_IN4 16

#define MIN_DISTANCE 20
#define MAX_DISTANCE 50
// minimum distance to trigger a backward maneuver (cm)
#define MIN_DISTANCE_BACK 5

float leftDist, centerDist, rightDist;

void setup() {
  Serial.begin(115200);
  
  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);
  pinMode(CENTER_TRIG, OUTPUT);
  pinMode(CENTER_ECHO, INPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);
  
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  
  stopMotors();
  delay(1000);
}

void loop() {
  readSensors();
  followHuman();
  delay(100);
}

float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long duration = pulseIn(echo, HIGH, 30000);
  return (duration == 0) ? 999 : (duration * 0.034) / 2;
}

void readSensors() {
  leftDist = getDistance(LEFT_TRIG, LEFT_ECHO);
  centerDist = getDistance(CENTER_TRIG, CENTER_ECHO);
  rightDist = getDistance(RIGHT_TRIG, RIGHT_ECHO);
}

void followHuman() {
  // Debug print
  Serial.print("L:");
  Serial.print(leftDist);
  Serial.print(" C:");
  Serial.print(centerDist);
  Serial.print(" R:");
  Serial.println(rightDist);

  if (centerDist < MIN_DISTANCE_BACK) {
    // Too close in front — back up a bit
    moveBackward();
    Serial.println("Action: backward");
  } else if (centerDist < leftDist && centerDist < rightDist && centerDist < MAX_DISTANCE) {
    // Front is the closest and within follow range
    moveForward();
    Serial.println("Action: forward");
  } else if (leftDist < rightDist && leftDist < MAX_DISTANCE) {
    // Turn toward left obstacle (prefer left)
    turnLeft();
    Serial.println("Action: left");
  } else if (rightDist < MAX_DISTANCE) {
    // Turn toward right obstacle
    turnRight();
    Serial.println("Action: right");
  } else {
    stopMotors();
    Serial.println("Action: stop");
  }
}

void moveForward() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
}

void turnLeft() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
}

void turnRight() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
}

void stopMotors() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}

void moveBackward() {
  // Reverse both motors
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
}