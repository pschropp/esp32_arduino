// platformio.io

// Import libraries
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ThingerWifi.h>
#include "DHT.h"
#include "arduino_secrets.h"

// Define and initialise the sensor
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Setup device details
ThingerWifi thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
	// Setup WiFi
	thing.add_wifi(SSID, SSID_PASSWORD);
	// Define the 'thing' with a name and data direction
	thing["dht22"] >> [](pson& out){
		// Add the values and the corresponding code
		out["humidity"] = dht.readHumidity();
		out["celsius"] = dht.readTemperature();
	};
}

void loop() {
  thing.handle();
}
