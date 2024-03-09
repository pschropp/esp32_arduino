#define THINGER_SERIAL_DEBUG

// Import libraries
#include <Arduino.h>
#include <WiFi.h>
#include <ThingerESP32.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "arduino_secrets.h"

// Define and initialise the sensor
Adafruit_HTU21DF sensor = Adafruit_HTU21DF();

// Setup device details
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  // open serial for debugging
  Serial.begin(115200);
  if (!sensor.begin()) {
    Serial.println("Did not find sensor!");
    while (true)
      ;
  }

	// Setup WiFi
	thing.add_wifi(WIFI_SSID, WIFI_PASSWORD);
	// Define the 'thing' with a name and data direction

	thing["HTU21D"] >> [](pson& out){
		// Add the values and the corresponding code
		out["temperature (°C)"] = sensor.readTemperature();
		out["humidity (%)"] = sensor.readHumidity();
	};
}

void loop() {
  float temp = sensor.readTemperature();
  Serial.print(F("temperature in C: ")); Serial.println(temp, 2);
  float hum  = sensor.readHumidity();
  Serial.print(F("humidity in %: ")); Serial.println(hum, 2);

  thing.handle();
  delay(2000);
}
