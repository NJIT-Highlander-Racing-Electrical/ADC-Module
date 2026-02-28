# Analog-to-Digital Converter (ADC) Module
Consolidated module for reading all analog sensors on the vehicle (ie. suspension displacement and brake pressure).
## About
This module utilizes two Adafruit ADS1015 breakout boards, with pads allocated for voltage dividers, to read analog inputs from various sensors. The data is then processed and sent over CAN Bus.
## Research
The ESP-32's onboard ADC is known to be temperamental. As a result, we are moving over to discrete ADC ics or breakout boards for the 2025-2026 season. We already had Adafruit ADS1015 breakout boards in stock, so those are the ones that got used. For future seasons, it would be best to integrate ADC ics directly onto the module's circuit board to reduce possible damage from vibrations.

For the 2025-2026 season, a lot of empty space in the vehicle which previously served as convenient places to put our modules. Thus, all modules with analog sensors were condensed into a single module.
## 2025-2026 Goals
- Consolidate all analog sensors into one box.
- Use discrete ADC ics/breakouts instead of ones on the ESP-32.
- Improve reliability of readings.
- Figure out how we can use smaller shock sensors/
