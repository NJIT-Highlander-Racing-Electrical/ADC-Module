/**********************************************
 * NJIT Highlander Racing
 * 8-Channel ADC
 * Author(s): Alexander Huegler, Andrew Santella
 * 2025-2026 Season
 **********************************************/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <BajaCAN.h>

 // Resistor Divider Values
#define R1 20000.0f
#define R2 43000.0f
#define DIVIDER_RATIO (R2 / (R1 + R2))

Adafruit_ADS1015 ads0; // Brake pressure  (ADDR → VCC = 0x49)
Adafruit_ADS1015 ads1; // Displacement    (ADDR → GND = 0x48)

/**********************************************
 * Suspension Potentiometer Calibration:
 *
 *  1. Rough pass. Set all calibration factors in COUNTS_PER_INCH[] to -1. Push
 *     sensors all the way in, upload code, and power on the system. Calibration
 *     factor will be calculated using the following formula:
 *     (counts_at_far - counts_at_near) / (far_inches - near_inches).
 *     Counts are reported in the serial output and inches are measured directly.
 *     Repeat for each sensor.
 *  2. Fine pass. Once each sensor is roughly calibrated, begin checking reported
 *     measurements against measurements taken by hand. Adjust calibration factors
 *     in small increments until measurements are as close as possible. They will
 *     likely never be perfect, as the measurment circuits may induce some
 *     non-linearity.
 *
 * Brake Transducer Calibration:
 *
 *  The transducer signal is run through a voltage (43k / (20k + 43k))divider
 *  before it is measured by the ADC.
 *
 *  1. Change the R1 and R2 macros to match ones actually used.
 *
**********************************************/

// Suspension Calibration Factors: { RR, FR, RL, FL}
const float COUNTS_PER_INCH[] = { -166.25, -166, -166.25, -166.25 };

// Suspension Zero Offsets (captured at startup)
// All displacement readings are relative to these startup counts!
int16_t zeroCount[4] = { 0, 0, 0, 0 };

// Function Declarations
void zeroSuspensionSensors();
void readDisplacementSensors();
void readBrakePressureSensors();
void serialDataOutput();
float rawToPSI(int16_t raw);

void setup() {
  Serial.begin(460800);
  while (!Serial);
  delay(500);

  if (!ads0.begin(0x49)) { Serial.println("ADS1015_0 (brake) not found!"); while (1); }
  if (!ads1.begin(0x48)) { Serial.println("ADS1015_1 (suspension) not found!"); while (1); }

  ads0.setGain(GAIN_ONE);  // ±4.096V, 2mV/count on ADS1015
  ads1.setGain(GAIN_ONE);

  zeroSuspensionSensors();
  setupCAN(WHEEL_SPEED, 10);
}

void loop() {
  readDisplacementSensors();
  readBrakePressureSensors();
  serialDataOutput();
  vTaskDelay(pdMS_TO_TICKS(100));
}

// Capture suspension sensor positions at startup to use as relative zero.
void zeroSuspensionSensors() {
  for (int i = 0; i < 4; i++) {
    zeroCount[i] = ads1.readADC_SingleEnded(i);
  }
  Serial.println("Suspension zeroed.");
  Serial.print("  Zero counts — FL:"); Serial.print(zeroCount[3]);
  Serial.print("  FR:"); Serial.print(zeroCount[1]);
  Serial.print("  RL:"); Serial.print(zeroCount[2]);
  Serial.print("  RR:"); Serial.println(zeroCount[0]);
}

// Positive = compression (shock pushed inward from zero).
// Negative = extension  (shock pulled outward from zero).
void readDisplacementSensors() {
  frontLeftDisplacement = (ads1.readADC_SingleEnded(3) - zeroCount[3]) / COUNTS_PER_INCH[3];
  frontRightDisplacement = (ads1.readADC_SingleEnded(1) - zeroCount[1]) / COUNTS_PER_INCH[1];
  rearLeftDisplacement = (ads1.readADC_SingleEnded(2) - zeroCount[2]) / COUNTS_PER_INCH[2];
  rearRightDisplacement = (ads1.readADC_SingleEnded(0) - zeroCount[0]) / COUNTS_PER_INCH[0];
}

void readBrakePressureSensors() {
  frontBrakePressure = rawToPSI(ads0.readADC_SingleEnded(0));
  rearBrakePressure = rawToPSI(ads0.readADC_SingleEnded(1));
}

// Convert raw counts to PSI
float rawToPSI(int16_t raw) {
  float vSensor = ads0.computeVolts(raw) / DIVIDER_RATIO;  // undo the divider
  float psi = ((vSensor - 0.5f) / 4.0f) * 2000.0f;
  return constrain(psi, 0.0f, 2000.0f);
}

// Serial Output
void serialDataOutput() {
  Serial.println("____________________________________________________________________");
  Serial.println("|    FL    |    FR    |    RL    |    RR    ||  FB PSI  |  RB PSI  |");
  Serial.println("|----------|----------|----------|----------||----------|----------|");
  Serial.printf("| %6.2f\"  | %6.2f\"  | %6.2f\"  | %6.2f\"  || %8d | %8d |\n",
    frontLeftDisplacement,
    frontRightDisplacement,
    rearLeftDisplacement,
    rearRightDisplacement,
    (int)frontBrakePressure,
    (int)rearBrakePressure);
  Serial.println("|----------|----------|----------|----------||----------|----------|");
}