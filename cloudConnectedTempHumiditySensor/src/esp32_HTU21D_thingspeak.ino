// Import libraries
#include <Arduino.h>
#include "arduino_secrets.h"
#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "ThingSpeak.h"

// for deep sleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  55        /* Time ESP32 will go to sleep (in seconds) */

const char wifi_ssid[] = WIFI_SSID;   // your network SSID (name) 
const char wifi_password[] = WIFI_PASSWORD;   // your network password
WiFiClient  client;

const unsigned long myChannelNumber = CHANNEL_ID;
const char * myWriteAPIKey = WRITE_APIKEY;
const char* server = "api.thingspeak.com";

Adafruit_HTU21DF sensor = Adafruit_HTU21DF();

void setup() {
  Serial.begin(115200);
  // wait for serial port to open
  while (!Serial) {
    delay(10);
  }
  Serial.println("Serial started");
  
  if (!sensor.begin()) {
    Serial.println("Did not find sensor!");
    while (true)
      ;
  }

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifi_ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(wifi_ssid, wifi_password);
      Serial.print(".");
      delay(4000);     
    } 
    Serial.print("Connected, ip address:\t");
    Serial.println(WiFi.localIP());
    Serial.println("---")
  }

  float temp = sensor.readTemperature();
  float hum = sensor.readHumidity();
  
  Serial.println("Temp: " + String(temp) + "°C");
  Serial.println("Humidity: " + String(hum) + "%");

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, hum);
  int x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
  if(x == 200){
    Serial.println("Data pushed successfully");
  }else{
    Serial.println("Push error" + String(x));
  }
  Serial.println("---");

  //delay(TIME_TO_SLEEP );  //delay not using deep sleep
  startDeepSleep();
}

void startDeepSleep(){
  Serial.println("Going to sleep...");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); 
  esp_deep_sleep_start();
}
