#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <BajaCAN.h>

Adafruit_ADS1015 ads0; // Use ADS1015 (12-bit) version
Adafruit_ADS1015 ads1; // Use ADS1015 (12-bit) version

// Calibration variables
// LPPS-20-250: max extension is 250 mm
const float calibrationFactor_frontLeft = 3.31;
const float calibrationFactor_frontRight = 3.1;
const float calibrationFactor_rearLeft = 3.1;
const float calibrationFactor_rearRight = 3.1;

// Function declarations
void serialDataOutput();
void readDisplacementSensors();
void readBrakePressureSensors();

void setup() {
  Serial.begin(115200);

  // Initialize I2C on the ESP32 (Default is 21 & 22)
  if (!ads0.begin(0x49)) {
    Serial.println("Failed to initialize ADS1015_0. Check wiring!");
    while (1);
  }
  else {
    Serial.println("Intialized ADS1015_0 successfully!");
  }
  if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS1015_1. Check wiring!");
    while (1);
  }
  else {
    Serial.println("Intialized ADS1015_1 successfully!");
  }

  // Set gain to +/- 4.096V (1 bit = 2mV in 12-bit mode)
  // Options: GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN
  ads0.setGain(GAIN_ONE);
  ads1.setGain(GAIN_ONE);

  // setup CAN
  setupCAN(WHEEL_SPEED);
}

void loop() {
  int16_t adc0;
  float volts;

  readDisplacementSensors();

  serialDataOutput();

  delay(100);
}

void serialDataOutput() {
  Serial.println("____________________________________________");
  Serial.println("|  FL  |  FR  |  RL  |  RR  ||  FB  |  RB  |");
  Serial.println("|------|------|------|------||------|------|");

  Serial.print("| ");
  Serial.print(frontLeftDisplacement);
  Serial.print(" | ");
  Serial.print(frontRightDisplacement);
  Serial.print(" | ");
  Serial.print(rearLeftDisplacement);
  Serial.print(" | ");
  Serial.print(rearRightDisplacement);
  Serial.print(" ||  ");
  Serial.print(frontBrakePressure);
  Serial.print("   |   ");
  Serial.print(rearBrakePressure);
  Serial.println("  |");

  Serial.println("|------|------|------|------||------|------|");
}

void readDisplacementSensors() {
  float adc_values[4];

  // Read all four inputs of ADC 1 and convert to volts
  for (int i = 0; i < 4; i++) {
    adc_values[i] = ads1.computeVolts(ads1.readADC_SingleEnded(i));
  }

  // Convert volts to cm for each sensor
  adc_values[0] = adc_values[0] / calibrationFactor_frontLeft * 25;
  adc_values[1] = adc_values[1] / calibrationFactor_frontRight * 25;
  adc_values[2] = adc_values[2] / calibrationFactor_rearLeft * 25;
  adc_values[3] = adc_values[3] / calibrationFactor_rearRight * 25;

  frontLeftDisplacement = adc_values[0];
  frontRightDisplacement = adc_values[1];
  rearLeftDisplacement = adc_values[2];
  rearRightDisplacement = adc_values[3];
}

void readBrakePressureSensors() {
  float adc_values[2];

  // Read first two inputs of ADC 0 and convert to volts
  for (int i = 0; i < 2; i++) {
    adc_values[i] = ads0.computeVolts(ads0.readADC_SingleEnded(i));
  }

  frontBrakePressure = adc_values[0];
  rearBrakePressure = adc_values[1];
}