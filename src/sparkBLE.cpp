#include <Wire.h>
#include <ArduinoBLE.h>
#include "SparkFun_STC3x_Arduino_Library.h"

STC3x mySensor;

// Define BLE service and characteristics for CO2 and temperature
BLEService sensorService("180C");
BLECharacteristic co2Characteristic("2A6E", BLERead | BLENotify, 20);
BLECharacteristic tempCharacteristic("2A6F", BLERead | BLENotify, 20);

// Variables for LED pulsing when not connected
const int yellowLedPin = 13;  // Built-in yellow/orange LED pin
unsigned long previousMillis = 0;
const long pulseInterval = 500;  // Interval at which to blink (500 ms)
bool ledState = LOW;

void setup() {
  Serial.begin(115200);  
  if (Serial) {
    while (!Serial);  
  }

  // Initialize the yellow/orange LED pin
  pinMode(yellowLedPin, OUTPUT);
  digitalWrite(yellowLedPin, LOW);  // Turn off LED initially

  Wire.begin();
  if (mySensor.begin() == false) {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1);
  }

  if (mySensor.setBinaryGas(STC3X_BINARY_GAS_CO2_AIR_25) == false) {
    Serial.println(F("Could not set the binary gas! Freezing..."));
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Nano33BLE_CO2");
  BLE.setAdvertisedService(sensorService);

  sensorService.addCharacteristic(co2Characteristic);
  sensorService.addCharacteristic(tempCharacteristic);
  BLE.addService(sensorService);

  BLE.advertise();
  Serial.println("BLE is now advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    // When connected, turn on the yellow/orange LED solid
    digitalWrite(yellowLedPin, HIGH);
    
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (mySensor.measureGasConcentration()) {
        float co2 = mySensor.getCO2();
        float temp = mySensor.getTemperature();

        // Print data in tab-separated format for Serial Plotter
        Serial.print(co2, 2);  // Print CO2
        Serial.print("\t");    // Tab character between values
        Serial.println(temp, 2);  // Print temperature and end with newline

        // Create buffers for the formatted strings
        char co2Str[20];
        char tempStr[20];

        // Format the CO2 and temperature as ASCII strings
        snprintf(co2Str, sizeof(co2Str), "CO2: %.2f%%", co2);
        snprintf(tempStr, sizeof(tempStr), "Temp: %.2fC", temp);

        // Send data via BLE characteristics as ASCII strings
        co2Characteristic.writeValue(co2Str);
        tempCharacteristic.writeValue(tempStr);
      }
      delay(0000);
    }

    Serial.println("Disconnected from central.");
  } else {
    // If not connected, pulse the yellow/orange LED
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= pulseInterval) {
      previousMillis = currentMillis;

      // If the LED is off, turn it on, and vice versa
      ledState = !ledState;
      digitalWrite(yellowLedPin, ledState);
    }
  }
}
