#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <BajaCAN.h>

Adafruit_ADS1015 ads; // Use ADS1015 (12-bit) version

// Calibration variables
// LPPS-20-250: max extension is 250 mm
const float calibrationFactor_frontLeft = 3.31;
const float calibrationFactor_frontRight = 3.1;
const float calibrationFactor_rearLeft = 3.1;
const float calibrationFactor_rearRight = 3.1;

// Function declarations
void readDisplacementSensors();

void setup()
{
  Serial.begin(115200);

  // Initialize I2C on the ESP32 (Default is 21 & 22)
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS1015. Check wiring!");
    while (1)
      ;
  }

  // Set gain to +/- 4.096V (1 bit = 2mV in 12-bit mode)
  // Options: GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN
  ads.setGain(GAIN_ONE);

  // setup CAN
  setupCAN(WHEEL_SPEED);
}

void loop()
{
  int16_t adc0;
  float volts;

  // Read from Channel 0 (A0 on the ADS1015 board)
  adc0 = ads.readADC_SingleEnded(0);
  volts = ads.computeVolts(adc0);

  // Serial.printf("ADS1015 A0 Raw: %d | Voltage: %.3fV\n", adc0, volts);

  readDisplacementSensors();

  delay(100);
}

void readDisplacementSensors()
{
  float adc_values[4];

  // Read all four inputs of ADC 0 and convert to volts
  for (int i = 0; i < 4; i++)
  {
    adc_values[i] = ads.computeVolts(ads.readADC_SingleEnded(i));
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

  Serial.println(adc_values[0]);
}