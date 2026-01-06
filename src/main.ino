// ESP32 Smart Farming Automation System
#define BLYNK_TEMPLATE_ID "TMPL3pxIfW0zI"
#define BLYNK_TEMPLATE_NAME "esp32 rover"
#define BLYNK_AUTH_TOKEN "Iu0551L2tieUp6yDiCkCwM7eu22RCW7e"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include "DHT.h"

char ssid[] = "Redmi 10";
char pass[] = "qwertyui";

// --- Motor driver pins ---
#define ENA 27
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26
#define ENB 14

// --- Sensor pins ---
#define SOIL_PIN 35
#define DHTPIN 4
#define DHTTYPE DHT11

// --- Servo pins ---
#define DIG_SERVO_PIN 13
#define DISPENSE_SERVO_PIN 5

Servo soilServo;
Servo seedServo;
DHT dht(DHTPIN, DHTTYPE);

// --- Global vars ---
int moisturePercent = 0;
int motorSpeed = 150;
bool digServoState = false;
bool seedServoState = false;

// --- Movement functions ---
void moveStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, motorSpeed);
}

void moveLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, motorSpeed);
}

void moveRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, 0);
}

// --- Digging Servo (V6) ---
void toggleDigServo() {
  if (!digServoState) {
    soilServo.write(90);  // dig down
    delay(9000);
    soilServo.write(0);   // retract
  } else {
    soilServo.write(0);
  }
  digServoState = !digServoState;
}

// --- Seed Dispenser Servo (V7) ---
void toggleSeedServo() {
  if (!seedServoState) {
    seedServo.write(90);  // open funnel
    delay(1000);
    seedServo.write(0);   // close
    Serial.println("🌾 Seed Dispensed");
  } else {
    seedServo.write(0);   // stays closed
  }
  seedServoState = !seedServoState;
}

// --- Blynk Buttons ---
BLYNK_WRITE(V0) { if (param.asInt()) moveForward(); else moveStop(); }
BLYNK_WRITE(V1) { if (param.asInt()) moveBackward(); else moveStop(); }
BLYNK_WRITE(V2) { if (param.asInt()) moveLeft(); else moveStop(); }
BLYNK_WRITE(V3) { if (param.asInt()) moveRight(); else moveStop(); }
BLYNK_WRITE(V4) { if (param.asInt()) moveStop(); }
BLYNK_WRITE(V6) { if (param.asInt()) toggleDigServo(); }
BLYNK_WRITE(V7) { if (param.asInt()) toggleSeedServo(); }

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(SOIL_PIN, INPUT);

  soilServo.attach(DIG_SERVO_PIN);
  soilServo.write(0);

  seedServo.attach(DISPENSE_SERVO_PIN);
  seedServo.write(0);

  dht.begin();
  moveStop();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("✅ Connected to Blynk and ready to roll!");
}

void loop() {
  Blynk.run();

  // --- Soil Moisture ---
  int soilVal = analogRead(SOIL_PIN);
  moisturePercent = map(soilVal, 3500, 1200, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  Blynk.virtualWrite(V5, moisturePercent);
  Serial.print("🌱 Soil Moisture: "); Serial.print(moisturePercent); Serial.println("%");

  // --- DHT11 Temp & Humidity ---
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius

  if (!isnan(humidity) && !isnan(temperature)) {
    Blynk.virtualWrite(V8, temperature);
    Blynk.virtualWrite(V9, humidity);
    Serial.print("🌡 Temp: "); Serial.print(temperature); Serial.print(" °C  |  💧 Humidity: ");
    Serial.print(humidity); Serial.println(" %");
  } else {
    Serial.println("⚠️ DHT11 read failed");
  }

  delay(1000);
}
