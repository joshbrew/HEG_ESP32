//Requires these arduino libraries: Arduino ESP32 latest dev or official build via git, AsyncTCP, ESPAsyncWebServer, Arduino ADS1X15 library, ArduinoJson
//By Joshua Brewster (MIT License)
//Contributors: Diego Schmaedech and many others indirectly. 

#include "WiFi_API.h" // WiFi settings and macros. HEG and BLE libraries linked through here.

//===============================================================
// Setup
//===============================================================
bool BLEtoggle = false;
bool toggleSleep = false;
bool WIFItoggle = false;
unsigned long bootMicros = 0;


void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  currentMicros = esp_timer_get_time();
  setupHEG();
  delay(20);
  startADS();
  delay(20);
  EEPROM.begin(512);
  int sleep = EEPROM.read(510);
  if(sleep == 1){
    EEPROM.write(510,0); //Disarm sleep toggle for next reset
    EEPROM.end();
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(300);
    esp_deep_sleep_start(); //Ends the loop() until device is reset.
  }
  int wifiReset = EEPROM.read(509);
  int toggle = EEPROM.read(511);
  int ComMode = EEPROM.read(0); //Communication mode (WiFi, BLE, or BT Serial)
  if(toggle == 1){
    if(ComMode == 1){
      ComMode = 0;
      EEPROM.write(0,0);
    }
    else if (ComMode == 0){
      ComMode = 2;
      EEPROM.write(0,2);
    } 
    else {
      ComMode = 1;
      EEPROM.write(0,1);
    }
  }
  EEPROM.write(510,1); //Arm the sleep toggle (first in order of checks)
  EEPROM.write(509,0); // Disarm the wifi login reset
  EEPROM.end();
  
  if(wifiReset == 1){
    saveWiFiLogin(true,false,false,false); //Reset WiFi credentials
  }

  //Now set up the communication protocols (Only 1 active at a time for best results!)
  if(ComMode == 1) {
     setupBLE();

     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
     delay(100);
     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
  }
  else if(ComMode == 2) {
     setupBTSerial();
     
     
     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
     delay(100);
     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
     delay(100);
     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
     delay(100);
     digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
     delay(100);
     digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
    
    
  }
  else if(ComMode == 3) {
    Serial.println("USB Only configuration.");
    Serial.println("Sample commands: 't': Toggle HEG program ON, 'f': Toggle HEG program OFF, 'u': Toggle WiFi mode, 'b': Toggle BLE mode, 'B': Toggle Bluetooth Serial mode");
  }
  else {
    setupWiFi();
    
    digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
    delay(100);
    digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
    delay(100);
    digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
    delay(100);
    digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
    delay(100);
    digitalWrite(13,HIGH);
     digitalWrite(5,LOW);
    delay(100);
    digitalWrite(13,LOW);
     digitalWrite(5,HIGH);
    
  }
  bootMicros = esp_timer_get_time();
  //xTaskCreate(HEG_core_loop, "HEG_core_loop", 16384, NULL, 1, NULL);
}

void toggleCheck(){ //Checks toggles on initialization
  if(currentMicros - bootMicros < 3500000){
    if((toggleSleep == false) && (currentMicros - bootMicros > 1000000)){
      EEPROM.begin(512);
      EEPROM.write(510,0); // Disarm the sleep toggle
      EEPROM.write(511,1); // Now arm the BLE/WiFi toggle
      EEPROM.end();
      toggleSleep = true;
      
      digitalWrite(13,HIGH);
      digitalWrite(5,LOW);
      delay(200);
      digitalWrite(13,LOW);
      digitalWrite(5,HIGH);
      
    }
    if((toggleSleep == true) && (BLEtoggle == false) && (currentMicros - bootMicros > 2000000) ){
      EEPROM.begin(512);
      EEPROM.write(511,0); //Disarm BLE/WiFi toggle
      EEPROM.write(509,1); // Now arm the Wifi reset toggle
      EEPROM.end();
      BLEtoggle = true;

      digitalWrite(13,HIGH);
      digitalWrite(5,LOW);
      delay(200);
      digitalWrite(13,LOW);
      digitalWrite(5,HIGH);
      
    }
    if((BLEtoggle == true) && (WIFItoggle == false) && (currentMicros - bootMicros > 3000000) ){
      EEPROM.begin(512);
      EEPROM.write(509,0); //Disarm BLE/WiFi toggle
      EEPROM.end();
      WIFItoggle = true;

      digitalWrite(13,HIGH);
      digitalWrite(5,LOW);
      delay(200);
      digitalWrite(13,LOW);
      digitalWrite(5,HIGH);
      
    }
  }
}

void checkInput()
{
  if (USE_BT == true)
  {
    while (SerialBT.available())
    {
      received = SerialBT.read();
      SerialBT.println(received);
      commandESP32(received);
      SerialBT.read(); //Flush endline for single char response
    }
  }
  if (USE_USB == true)
  {
    while (Serial.available())
    {
      received = Serial.read();
      Serial.println(received);
      commandESP32(received);
      Serial.read();
    }
  }
}

void loop(void){
  currentMicros = esp_timer_get_time();

  toggleCheck();
  
  //Serial.print("Time (ms): ");
  //Serial.println(currentMillis);
  //delayMicroseconds(500);
  HEG_core_loop();
  if(currentMicros - inputMicros >= 300000){ // Check input every N microseconds
    inputMicros = currentMicros;
    delayMicroseconds(1800);
    checkInput();
    //delayMicroseconds(1000); // Hotfix for checkInput not working without delay.
  }
}
