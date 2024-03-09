#define THINGER_SERIAL_DEBUG

// Import libraries
#include <Arduino.h>
#include <WiFi.h>
#include <ThingerESP32.h>
#include "DHT.h"
#include "arduino_secrets.h"

// Define and initialise the sensor
#define DHTPIN 25
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Setup device details
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  // open serial for debugging
  Serial.begin(115200);
  dht.begin();

	// Setup WiFi
	thing.add_wifi(SSID, SSID_PASSWORD);
	// Define the 'thing' with a name and data direction

	thing["dht22"] >> [](pson& out){
		// Add the values and the corresponding code
		out["humidity (%)"] = dht.readHumidity();
		out["temperature (°C)"] = dht.readTemperature();
	};
}

void loop() {
  float temp = dht.readTemperature();
  Serial.print(F("temperature in C: ")); Serial.println(temp);
  float hum  = dht.readHumidity();
  Serial.print(F("humidity in %: ")); Serial.println(hum);

  thing.handle();
  delay(5000);
}
