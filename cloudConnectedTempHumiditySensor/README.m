# ESP32 Temperature + Humidity to Cloud
## Setup
### Board selection in Arduino
choose ESP32 , then for Devkit C choose ESP32 dev

### USB
Windows driver: go to device manager, see ports, install driver for connected esp: https://www.silabs.com/documents/public/software/CP210x_Universal_Windows_Driver.zip

### I2C Connection
include Wire.h, connect SDA to pin 21 and SCL to pin 22 on Devkit C V4

## Sensor
HTU21D temperature and humidity sensor. Libraries: HTU21DF by Adafruit.
