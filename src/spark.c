//Modified SparkFun code including self- calibration. 

/*
  Reading CO2 and temperature from the STC3x
  This example demonstrates how to perform regular calibration

  Hardware Connections:
  - Attach Arduino to computer using a USB cable.
  - Connect STC3x sensor to Arduino using appropriate wiring.
  - Open Serial Monitor at 115200 baud.
*/

#include <Wire.h>
#include "SparkFun_STC3x_Arduino_Library.h"

STC3x mySensor;
static unsigned long lastCO2Time = 0;
float co2Value = 0;  // Stores the last CO2 value


void setup()
{
  Serial.begin(9600);
  Serial.println(F("STC3x Calibration"));
  Wire.begin();

  //mySensor.enableDebugging(); // Enable debugging messages

  if (!mySensor.begin())
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1);
  }
  //Possible values are:
  //  STC3X_BINARY_GAS_CO2_N2_100   : Set binary gas to CO2 in N2.  Range: 0 to 100 vol%
  //  STC3X_BINARY_GAS_CO2_AIR_100  : Set binary gas to CO2 in Air. Range: 0 to 100 vol%
  //  STC3X_BINARY_GAS_CO2_N2_25    : Set binary gas to CO2 in N2.  Range: 0 to 25 vol%
  //  STC3X_BINARY_GAS_CO2_AIR_25   : Set binary gas to CO2 in Air. Range: 0 to 25 vol%
  if (!mySensor.setBinaryGas(STC3X_BINARY_GAS_CO2_AIR_25))
  {
    Serial.println(F("Could not set the binary gas! Freezing..."));
    while (1);
  }

  /*// Sensor warm-up
  Serial.println(F("Warming up sensor..."));
  delay(60000); // Wait for 1 minute

  // Stabilization in calibration gas
  Serial.println(F("Place sensor in calibration gas now."));
  delay(300000); // Wait 5 minutes for stabilization
*/
  // Perform forced recalibration
  float referenceConcentration = 0.00; // 0.0 ppm CO2
  if (mySensor.forcedRecalibration(referenceConcentration))
  {
    Serial.println(F("Forced Recalibration was successful!"));
  }
  else
  {
    Serial.println(F("Forced Recalibration FAILED!"));
  }

  // Enable automatic self-calibration
  if (mySensor.enableAutomaticSelfCalibration())
  {
    Serial.println(F("Automatic self-calibration is enabled!"));
  }
  else
  {
    Serial.println(F("EnableAutomaticSelfCalibration FAILED!"));
  }
}


void loop()
{
  unsigned long currentTime = millis();

  // Measure as often as possible
  if (mySensor.measureGasConcentration())
  {
    float temperature = mySensor.getTemperature();

    // Update CO2 value every 10 seconds
    if (currentTime - lastCO2Time >= 10000)
    {
      lastCO2Time = currentTime;
      co2Value = mySensor.getCO2();
    }

    // Output temperature and CO2 value for plotting
    Serial.print(F("\tTemperature(C):"));
    Serial.print(temperature, 2);  // Temperature in Celsius
    Serial.print(" ");
    Serial.print(F("CO2(%):"));
    Serial.println(co2Value, 2);   // CO2 concentration in %
    Serial.println();
  }
  else 
    Serial.print(F("."));

  delay(100);  // 1Hz
}

// 9600 Baud
