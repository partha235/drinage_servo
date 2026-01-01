#include <NewPing.h>
#include <Servo.h>

// Pin definitions
#define TRIG_PIN D4       // GPIO2
#define ECHO_PIN D2       // GPIO0
#define BUZZER_PIN D5     // GPIO14
#define SERVO_PIN D6      // GPIO12

#define MAX_DISTANCE 300  // Maximum distance for HC-SR04 (in cm)
#define DETECTION_THRESHOLD 5
#define SERVO_OPEN_ANGLE 90
#define SERVO_CLOSED_ANGLE 0

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Servo myServo;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Make sure buzzer is off initially
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_CLOSED_ANGLE); // Close servo initially
  Serial.begin(9600);
  Serial.println("Booting...");  // Add this line

}

void loop() {
  unsigned int distance = sonar.ping_cm();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance < DETECTION_THRESHOLD) {
    tone(BUZZER_PIN, 1000);               // Start buzzer tone
    myServo.write(SERVO_OPEN_ANGLE);     // Move servo to 90°
    delay(500);
  } else {
    noTone(BUZZER_PIN);                  // Stop buzzer
    myServo.write(SERVO_CLOSED_ANGLE);   // Move servo back to 0°
  }

  delay(100);
}
