/* 
ESP32 clean pattern for arduino websockets server to control GPIOs.

includes following "features":
 *  initiates both Wifi connection to a router (StationMode) and offering a softAP. mDNS: <mDNSname>.local
 *  SPIFFS file storage. listing SPIFFS content in serial monitor when starting or when handling files.
 *  SPIFFS is used for serving the necessary .html, .js and .css files
 *  file upload to SPIFFS via <IP>/edit.html or <mDNSname>.local/edit.html
 *  OTA implemented
 * (example: turn on button (not in html, should trigger js function sendOn() there) sends "on", webSocketEvent() reads this and triggers wrtiting PIN)
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>

#include <FS.h>
#include <SPIFFS.h>

#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>

#include "arduino_secrets.h"


/*----------------- configure Wifi -------------*/
WiFiMulti wifiMulti;                  // Create an instance of the WiFiMulti class, called 'wifiMulti'
const char* ssid = WIFI_SSID;         // The name of the Wi-Fi AP network that will be created
const char* password = WIFI_PASSWORD;    // The password (minimum 8 char!) required to connect to it, leave blank for an open network

const char* OTAName = OTA_SSID;        // A name and a password for the OTA service
const char* OTAPassword = OTA_PASSWORD;

const char* mdnsName = "esp32";       // Domain name for the mDNS responder

/* --------------- initialize servers ----------*/
WebServer server(80);                 // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81);       // create a websocket server on port 81

/* --------------- global variables ----------*/
File fsUploadFile;                    // a File variable to temporarily store the received file
// add project specific global variables below
// #define LED     15                // specify the pins with an LED connected


void setup() {
  pinMode(LED, OUTPUT);    // the pins with LEDs connected are outputs

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  
  startOTA();                  // Start the OTA service
  
  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server
  
  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler

}


// unsigned long prevMillis = millis();    //for interval check

void loop() {
  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events

/* if sth. should be done after certain intervals; events (like receiving text) is handled by the function webSocketEvent() below
  if(millis() > prevMillis + 32) {  
    // do sth.        
    prevMillis = millis();
  }
}
*/

/* -------------------Setup Functions (called above in Setup) ------------------------------ */
void startWiFi() {                    // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.softAP(ssid, password);        // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  wifiMulti.addAP("wifiSSID", "wifipassword");   // add Wi-Fi networks you want to connect to
  // wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  // wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {       // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  if(WiFi.softAPgetStationNum() == 0) {      // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP32 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP32 AP");
  }
  Serial.println("\r\n");
}

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    // digitalWrite(LED_RED, 0);    // turn off the LEDs, project specific
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startSPIFFS() {              // Start the SPIFFS and list all contents
  SPIFFS.begin();                   // Start the SPI Flash File System (SPIFFS)
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS started. Contents:");  // from here on only to list content of SPIFFS
  File root = SPIFFS.open("/");         // this will return a File object, even though there is no file with this name on the file system. will represent root.
  File file = root.openNextFile();      // returns another File object, now representing an actual file.
  while(file){
      String fileName = file.name();
      size_t fileSize = file.size();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
      file = root.openNextFile();
  }
  Serial.printf("\n");
}

void startWebSocket() {               // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() {                            // Start the mDNS responder
  MDNS.begin(mdnsName);                         // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startServer() {                          // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {    // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", ""); 
  }, handleFileUpload);                         // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);            // if someone requests any other file or page, go to function 'handleNotFound'
                                                // and check if the file exists

  server.begin();                               // start the HTTP server
  Serial.println("HTTP server started.");
}



/* ----------------- Server Handlers ---------------------------- */
void handleNotFound(){                        // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) {                    // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){                          // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if(upload.status == UPLOAD_FILE_START){
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.endsWith(".gz")) {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");                // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {     // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:                           // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {                            // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break; 
    case WStype_TEXT:                                   // if new text data is received // message handling case must be implemented.
      Serial.printf("[%u] get Text: %s\n", num, payload);
      // project specific reactions to payload below. call functions that are declared in section HELPER_FUNCTIONS.
      /*
      if (payload == 'on') {                          // read payload and do accordant logic, e.g. call accordant functions
        digitalWrite(LED, HIGH);                         // write High to the LED output pin, i.e. turn on
      } else if (payload == 'off') {                      // the browser sends an R when the rainbow effect is enabled
        digitalWrite(LED, LOW);                       // write LOW to the LED output pin, i.e. turn off
      }
      break;
      */
  }
}


/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/
// general functions needed for file handling
String formatBytes(size_t bytes) {              // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) {                // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

// project specific functions, esp. the ones called by webSocketEvent() case WStype_TEXT, below
