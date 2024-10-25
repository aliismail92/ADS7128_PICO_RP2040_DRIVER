ADS7128 ADC driver for the PICO/RP2040 mcu.

The library supports the following functions:
- Analog read of any ADC channel in manual mode (specify which channel to read) returning 12bit values
- Enabling over sampling rate of the sensor returning 16bit values
- Sensor calibration
- Setting any/multiple GPIO's as digital input/output to read and write digital signal

Missing features:
- Interrupts
- ZSD and threshold GPIO switching
- RMS and CRC

The code above contains an example with a working code to read from an analog channel, write to a digital channel, and read from a digital channel

