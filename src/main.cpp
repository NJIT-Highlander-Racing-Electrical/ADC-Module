/*
   ===============================================================
   ESP32 + ADS1015 + Piher Hall Sensor  (Manual Calibration)
   

   ⚙️  HOW TO USE:
   before following instruction run to see if it is to liking.






   
      1. Upload this code in PlatformIO and open Serial Monitor (115200 baud).
      2. Type 'm' → enter measure mode.
      3. Slowly rotate the magnet a full 360° once.
      4. Type 'q' → exit; copy the V_MIN / V_MAX values printed.
      5. Paste them into the constants near the top and re-upload.
      6. Type 'z' later to set the current position as 0° if desired.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// ----------- I²C pins (ESP32 defaults) -----------
#ifndef I2C_SDA
#define I2C_SDA 21
#endif
#ifndef I2C_SCL
#define I2C_SCL 22
#endif

Adafruit_ADS1015 ads;                // 12-bit ADC
const float DIV_RATIO = 1.0f / 3.0f; // 20 kΩ : 10 kΩ divider

// ----------- Manual calibration values -----------
//  🔧 Replace these after running measure mode.
float V_MIN = 0.474;   // lowest Vsensor over full turn
float V_MAX = 4.266;   // highest Vsensor over full turn

// ----------- Optional runtime zero offset -----------
float angle_offset_deg = 0.0f;

// ----------- Simple averaging filter -----------
const int AVG_N = 8;
int16_t ring[AVG_N];
int ringIdx = 0;
bool ringFilled = false;

// ===================================================
// Helper: read average voltage at ADS A0
// ===================================================
float readAdcVolts() {
  int16_t raw = ads.readADC_SingleEnded(0);
  float v = ads.computeVolts(raw);
  ring[ringIdx++] = raw;
  if (ringIdx >= AVG_N) { ringIdx = 0; ringFilled = true; }
  if (ringFilled) {
    long sum = 0;
    for (int i = 0; i < AVG_N; ++i) sum += ring[i];
    v = ads.computeVolts(sum / (float)AVG_N);
  }
  return v;
}

// ===================================================
// Helper: convert voltage to angle using V_MIN/V_MAX
// ===================================================
float mapToAngle(float v_sensor) {
  float span = V_MAX - V_MIN;
  if (span < 0.01f) return 0.0f;
  float angle = 360.0f * (v_sensor - V_MIN) / span;
  angle -= angle_offset_deg;             // apply zero offset
  while (angle < 0) angle += 360.0f;
  while (angle >= 360) angle -= 360.0f;
  return angle;
}

// ===================================================
// Measure mode: find your actual V_MIN / V_MAX
// ===================================================
void measureMode() {
  Serial.println("\n[MEASURE] Rotate the magnet through 360°. Press 'q' to finish.");
  float vmin = 10.0f;
  float vmax = -10.0f;

  while (true) {
    // exit on 'q'
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'q' || c == 'Q') break;
    }

    float v_adc = readAdcVolts();
    float v_sensor = v_adc / DIV_RATIO;
    if (v_sensor < vmin) vmin = v_sensor;
    if (v_sensor > vmax) vmax = v_sensor;

    Serial.print("Vsensor=");
    Serial.print(v_sensor, 3);
    Serial.print("  [min=");
    Serial.print(vmin, 3);
    Serial.print("  max=");
    Serial.print(vmax, 3);
    Serial.println("]");
    delay(30);
  }

  Serial.println("[MEASURE] Done.");
  Serial.print("👉  Use these values in code:\nV_MIN = ");
  Serial.print(vmin, 3);
  Serial.print(";  V_MAX = ");
  Serial.print(vmax, 3);
  Serial.println(";");
}

// ===================================================
// Standard Arduino setup()
// ===================================================
void setup() {
  Serial.begin(115200);
  delay(250);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!ads.begin(0x48)) {
    Serial.println("ERROR: ADS1015 not found at 0x48 (ADDR must be tied to GND).");
    while (true) delay(10);
  }

  ads.setGain(GAIN_ONE);                 // ±4.096 V range
  ads.setDataRate(RATE_ADS1015_3300SPS); // 3.3 kS/s max

  Serial.println("\n=== Hall Angle Reader (Manual Calibration) ===");
  Serial.println("Commands:");
  Serial.println("  m → measure V_MIN/V_MAX  (rotate then 'q' to exit)");
  Serial.println("  z → zero current angle");
  Serial.println("  h → help");
  Serial.println("After measuring, edit V_MIN/V_MAX at top & re-upload.");
}

// ===================================================
// Standard Arduino loop()
// ===================================================
void loop() {
  // ---- check for keyboard commands ----
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'm' || c == 'M') measureMode();
    else if (c == 'z' || c == 'Z') {
      // make current angle = 0°
      float v_sensor = readAdcVolts() / DIV_RATIO;
      float span = V_MAX - V_MIN;
      float angle_now = 360.0f * (v_sensor - V_MIN) / span;
      while (angle_now < 0) angle_now += 360.0f;
      while (angle_now >= 360) angle_now -= 360.0f;
      angle_offset_deg = angle_now;
      Serial.print("[ZERO] Current position set as 0°. Offset = ");
      Serial.println(angle_offset_deg, 2);
    } else if (c == 'h' || c == 'H') {
      Serial.println("Commands: m (measure)  q (quit measure)  z (zero)  h (help)");
    }
  }

  // ---- main reading loop ----
  float v_adc = readAdcVolts();
  float v_sensor = v_adc / DIV_RATIO;
  float angle = mapToAngle(v_sensor);

  Serial.print("Vadc=");
  Serial.print(v_adc, 3);
  Serial.print("  Vsensor=");
  Serial.print(v_sensor, 3);
  Serial.print(" V  Angle=");
  Serial.print(angle, 1);
  Serial.println("°");

  delay(40); // ~25 Hz update
}
