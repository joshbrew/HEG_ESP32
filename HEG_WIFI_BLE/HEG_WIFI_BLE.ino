//Requires these arduino libraries: Arduino ESP32 latest dev or official build via git, AsyncTCP, ESPAsyncWebServer, SavLay Filter, Arduino ADS1X15, and the different build options provided.
//By Joshua Brewster
//Contributors: Diego Schmaedech and many others indirectly. 

#include "WiFi_API.h" // WiFi settings and macros. HEG and BLE libraries linked through here.
unsigned long eventMillis = 0;

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
}


void loop(void){
  checkInput();
  HEG_core_loop();
  delayMicroseconds(1800);

  if(currentMillis - eventMillis >= 50){
    eventMillis = currentMillis;

    events.send(output.c_str(),"myevent",millis());
  }
}
