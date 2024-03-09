# ESP32 Websockets Pattern
## Table of Contents

1. [Usage](#usage)
2. [Motivation and Features](#motivation)
3. [Repository Structure](#structure)
4. [License and Acknowledgements](#license)


### Usage <a name="usage"></a>
1. Deploy .ino source code to ESP.
2. Upload files to server. To decrease server load and amount of data being sent, css, html and js files can be gzipped (gz) before being uploaded to server (SPIFFS). Do not compress images (jpeg, png, etc.).
3. Connect to server via browser


### Motivation and Features <a name="motivation"></a>
Empty pattern for websockets server for ESP32 Arduino controlling GPIOs

Features:
 *  initiates both Wifi connection to a router (StationMode) and offers a softAP. mDNS: <mDNSname>.local
 *  SPIFFS file storage. Lists SPIFFS content in serial monitor when starting or when handling files.
 *  SPIFFS is used for serving the necessary .html, .js and .css files
 *  file upload to SPIFFS via <IP>/edit.html or <mDNSname>.local/edit.html
 *  OTA


### Repository Structure <a name="structure"></a>
- this README file
- ESP32_websockets_emptyPattern.ino: Arduino source code
- data_non-gz: unzipped files (.html, .css, .js, .png, .ico) to upload to ESP for running the server
- .gitignore file to keep the repo clean


### License and Acknowledgements <a name="license"></a>
MainSource: https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html; 

Additional Source: https://github.com/tttapa/ESP8266/tree/master/Examples/14.%20WebSocket/A-WebSocket_LED_control
