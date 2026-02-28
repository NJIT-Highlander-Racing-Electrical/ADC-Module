#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;     // Use ADS1015 (12-bit) version

void setup() {
  Serial.begin(115200);

  // Initialize I2C on the ESP32 (Default is 21 & 22)
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1015. Check wiring!");
    while (1);
  }

  // Set gain to +/- 4.096V (1 bit = 2mV in 12-bit mode)
  // Options: GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN
  ads.setGain(GAIN_ONE); 
}

void loop() {
  int16_t adc0;
  float volts;

  // Read from Channel 0 (A0 on the ADS1015 board)
  adc0 = ads.readADC_SingleEnded(0);
  volts = ads.computeVolts(adc0);

  Serial.printf("ADS1015 A0 Raw: %d | Voltage: %.3fV\n", adc0, volts);

  delay(100);
}