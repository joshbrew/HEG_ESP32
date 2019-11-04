//Requires these arduino libraries: Arduino ESP32 latest dev or official build via git, AsyncTCP, ESPAsyncWebServer, SavLay Filter, Arduino ADS1X15, ArduinoJson
//By Joshua Brewster
//Contributors: Diego Schmaedech and many others indirectly. 

#include "WiFi_API.h" // WiFi settings and macros. HEG and BLE libraries linked through here.


//===============================================================
// Setup
//===============================================================

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  setupHEG();
  delay(100);
  startADS();
  delay(100);
  EEPROM.begin(512);
  if(EEPROM.read(0) != 1){
    EEPROM.end();
    setupWiFi();
  }
  else {
    EEPROM.end();
    setupBLE();
  }
  //xTaskCreate(HEG_core_loop, "HEG_core_loop", 16384, NULL, 1, NULL);
}


void loop(void){
  currentMillis = esp_timer_get_time() * 0.001;
  //Serial.print("Time (ms): ");
  //Serial.println(currentMillis);
  delayMicroseconds(1800);
  HEG_core_loop();
  if(currentMillis - inputMillis >= 300){ // Check input every N milliseconds
    inputMillis = currentMillis;
    checkInput();
    //delayMicroseconds(1000); // Hotfix for checkInput not working without delay.
  }

}
