//Requires these arduino libraries: Arduino ESP32 latest dev or official build via git, AsyncTCP, ESPAsyncWebServer, Arduino ADS1X15 library, ArduinoJson
//By Joshua Brewster (MIT License)
//Contributors: Diego Schmaedech and many others indirectly.

#include "WiFi_API.h" // WiFi settings and macros. HEG and BLE libraries linked through here.


//===============================================================
// Setup
//===============================================================

void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  currentMicros = esp_timer_get_time();
  setupHEG();
  delay(100);
  startADS();
  delay(100);

  EEPROM.begin(512);
  int ComMode = EEPROM.read(0);
  EEPROM.end();
  if (ComMode == 1) {
    setupBLE();
  }
  else if (ComMode == 2) {
    setupBTSerial();
  }
  else {
    setupWiFi();
  }
  //xTaskCreate(HEG_core_loop, "HEG_core_loop", 16384, NULL, 1, NULL);
}


void loop(void) {
  currentMicros = esp_timer_get_time();
  //Serial.print("Time (ms): ");
  //Serial.println(currentMillis);
  //delayMicroseconds(500);
  HEG_core_loop();
  if (currentMicros - inputMicros >= 300000) { // Check input every N microseconds
    inputMicros = currentMicros;
    delayMicroseconds(1800);
    checkInput();
    //delayMicroseconds(1000); // Hotfix for checkInput not working without delay.
  }
}
