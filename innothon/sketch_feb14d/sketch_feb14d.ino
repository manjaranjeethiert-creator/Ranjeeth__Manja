#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

RTC_DS3231 rtc;  // RTC object

#define RELAY_PIN 7  // LOW-LEVEL TRIGGER RELAY
SoftwareSerial BTSerial(10, 11); // RX, TX (Connect HC-05 TX to pin 10, RX to pin 11)

char receivedChar;
bool newData = false;
bool bluetoothConnected = false;
bool relayState = false;  // Store relay state (false = ON, true = OFF)
unsigned long lastBTCheck = 0;
const unsigned long btTimeout = 60000; // 10 seconds Bluetooth timeout

void setup() {
  Serial.begin(9600);      // Serial Monitor
  BTSerial.begin(9600);    // Bluetooth Module
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Default ON (6 PM - 6 AM behavior)

  Serial.println("System Starting...");
  Serial.println("Waiting for Bluetooth connection...");

  // RTC Initialization
  if (!rtc.begin()) {
    Serial.println("ERROR: Couldn't find RTC! System Halted.");
    while (1);
  } else {
    Serial.println("RTC Module Detected.");
  }

  // Self-Test for RELAY_PIN (D7)
  Serial.println("Testing RELAY_PIN (D7)...");
  digitalWrite(RELAY_PIN, LOW);
  delay(500);
  if (digitalRead(RELAY_PIN) == LOW) {
    Serial.println("✅ D7 is WORKING (LOW detected - Relay ON)");
  } else {
    Serial.println("❌ ERROR: D7 is NOT responding (LOW failed)");
  }

  digitalWrite(RELAY_PIN, HIGH);
  delay(500);
  if (digitalRead(RELAY_PIN) == HIGH) {
    Serial.println("✅ D7 is WORKING (HIGH detected - Relay OFF)");
  } else {
    Serial.println("❌ ERROR: D7 is NOT responding (HIGH failed)");
  }

  // Set initial state based on RTC
  DateTime now = rtc.now();
  relayState = !(now.hour() >= 18 || now.hour() < 6);  // *False (ON) between 6 PM - 6 AM, True (OFF) otherwise*
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
}

void loop() {
  // *Check for Bluetooth Data*
  if (BTSerial.available() > 0) {
    receivedChar = BTSerial.read();
    newData = true;
    bluetoothConnected = true;
    lastBTCheck = millis(); // Reset Bluetooth timeout
  }

  // *Check if Bluetooth is Inactive*
  if (millis() - lastBTCheck > btTimeout) {
    if (bluetoothConnected) {
      Serial.println("⚠ WARNING: No Bluetooth activity detected! Reverting to previous relay state.");
      bluetoothConnected = false; // Mark Bluetooth as disconnected
    }
  }

  // *Process Bluetooth Command*
  if (newData) {
    Serial.print("📡 Bluetooth Command Received: ");
    Serial.println(receivedChar);

    if (receivedChar == '1') {
      relayState = false;  // *ON*
    } else if (receivedChar == '0') {
      relayState = true;   // *OFF*
    }

    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);  // Apply relay state
    Serial.println(relayState ? "💡 Light Status: OFF (Bluetooth Command)" : "💡 Light Status: ON (Bluetooth Command)");
    newData = false;
  }

  // *RTC-based Automatic Control (only if Bluetooth is inactive)*
  if (!bluetoothConnected) {
    DateTime now = rtc.now();
    bool rtcState = !(now.hour() >= 18 || now.hour() < 6); // *False (ON) between 6 PM - 6 AM, True (OFF) otherwise*

    // RTC mode should only take over if Bluetooth is inactive, 
    // and should not change the state if the relay was on/off previously.
    if (rtcState != relayState) {
      relayState = rtcState; // Apply RTC logic (ON or OFF based on time)
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);  // Apply relay state based on RTC logic
    }

    // Print the current RTC time and relay state
    Serial.print("🕒 RTC Time: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.print(now.second());
    Serial.println(rtcState ? " -> Night Mode: ON" : " -> Night Mode: OFF");

    Serial.println(relayState ? "💡 Light Status: OFF (RTC Mode)" : "💡 Light Status: ON (RTC Mode)");
  } else {
    // *Bluetooth Connected: Keep Light in Last Commanded State*
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    Serial.println("📡 Bluetooth Connected: Keeping Last Commanded State.");
  }

  delay(100); // Print status every5 seconds
}
