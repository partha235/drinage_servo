#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NewPing.h>
#include <Servo.h>

// WiFi credentials
const char* ssid = "hello";
const char* password = "hello12345";

// Twilio credentials
const char* accountSid = "ACa4b5309188fdf608d27c6b32f7157388";
const char* authToken = "64b323e9cffc6dc5bf5b569c8e3c986a";
const char* twilioNumber = "+17623860321"; // Format: +1234567890
const char* recipientNumber = "+918778514160"; // Format: +1234567890

// Twilio API endpoint
const char* twilioApi = "https://api.twilio.com/2010-04-01/Accounts/";

// Pin definitions
#define TRIG_PIN D4       // GPIO2
#define ECHO_PIN D2       // GPIO0
#define BUZZER_PIN D5     // GPIO14
#define SERVO_PIN D6      // GPIO12

#define MAX_DISTANCE 300  // Maximum distance for HC-SR04 (in cm)
#define DETECTION_THRESHOLD 5
#define SERVO_OPEN_ANGLE 90
#define SERVO_CLOSED_ANGLE 0
#define SMS_COOLDOWN 60000 // 60 seconds cooldown for SMS

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Servo myServo;

unsigned long lastSMSTime = 0; // Track time of last SMS

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");

  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_CLOSED_ANGLE); // Close servo initially

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned int distance = sonar.ping_cm();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance < DETECTION_THRESHOLD) {
    tone(BUZZER_PIN, 1000);               // Start buzzer tone
    myServo.write(SERVO_OPEN_ANGLE);     // Move servo to 90°
    sendSMS("Critical distance detected! Object within " + String(distance) + " cm.");
    lastSMSTime = millis();
    delay(500);
  } else {
    noTone(BUZZER_PIN);                  // Stop buzzer
    myServo.write(SERVO_CLOSED_ANGLE);   // Move servo back to 0°
  }

  delay(100);
}

void sendSMS(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Note: Skips SSL verification for simplicity. Use proper certificates in production.
    
    HTTPClient http;
    
    // Construct Twilio API URL
    String url = String(twilioApi) + String(accountSid) + "/Messages.json";
    
    // Prepare POST data
    String postData = "To=" + String(recipientNumber) + "&From=" + String(twilioNumber) + "&Body=" + message;

    // Start HTTP client
    http.begin(client, url);
    http.setAuthorization(accountSid, authToken);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send POST request
    int httpCode = http.POST(postData);

    // Check response
    if (httpCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        Serial.println("SMS sent successfully!");
        Serial.println(http.getString());
      } else {
        Serial.println("Failed to send SMS");
        Serial.println(http.getString());
      }
    } else {
      Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    // Clean up
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}