// Joshua Brewster - HEG BTSerial
// Requires arduino ADS1x15 library and arduino esp32 library
/*
 * TODO
 * - HRV basic calculation, adjust LED flash rate accordingly (50ms is viable)
 * - Optimize memory usage. Take better advantage of ESP32.
 * - More data modes, transmitter modes (e.g. wifi vs bluetooth) might have to stay in separate sketches to preserve esp32 memory. The 16MB ESP32 might not have any trouble there.
 */

#include "BluetoothSerial.h"

#include <Wire.h>
#include <Adafruit_ADS1015.h>
//#include <esp_bt.h>
//#include <ArduinoJson.h>
 
BluetoothSerial SerialBT;

bool USE_USB = true; // WRITE 'u' TO TOGGLE, CHANGE HERE TO SET DEFAULT ON POWERING THE DEVICE
bool USE_BLUETOOTH = true; // WRITE 'b' TO TOGGLE
bool pIR_MODE = false; // SET TO TRUE OR WRITE 'p' TO DO PASSIVE INFRARED ONLY (NO RED LIGHT FOR BLOOD-OXYGEN DETECTION). RATIO IS USELESS HERE, USE ADC CHANGES AS MEASUREMENT.

bool DEBUG = false;
bool DEBUG_ADC = false; // FOR USE IN ARDUINO IDE WITH VIEW_ADC_VALUE
bool VIEW_ADC_VALUE = false; // FOR USE WITH ARDUINO IDE (SEE BELOW USAGE)
bool SEND_DUMMY_VALUE = false;


// HEG VARIABLES
int count = 0;
bool sensorEnabled = false;
bool adcEnabled = false;
bool reset = false;

char received;

// PUT IR IN 13, RED IN 12
const int IR = 13;
const int RED = 12;
const int LED = 5; // Lolin32 V1.0.0 LED on Pin 5
int16_t adc0; // Resulting 16 bit integer

//Setup ADS1115
Adafruit_ADS1115 ads;

float Voltage = 0.0;
float range = 32767; // 16 bit ADC (15 bits of range minus one)
float gain = 0.256; // +/- V
float bits2mv = gain / range;

//Signal flags
bool red_led = false; // Bool to alternate LEDS
bool badSignal = false; // Bool for too high of an ADC reading
bool signalDetermined = false; // Bool for whether the ADC reading is within desired range

//Counters
int ticks0, ticks1, ticks2, ticks3, ticks4 = 0;
int bticks3, bticks4 = 0;
//Scoring variables
long redValue = 0;
long irValue = 0;
float redAvg = 0;
float irAvg = 0;
float ratio = 0;
float baseline = 0;

//float score = 0;

float p1, p2 = 0;
float v1, v2 = 0;
float accel = 0;

float ratioAvg, adcAvg, posAvg; //velAvg, accelAvg;
float bratioAvg, badcAvg, bposAvg; //bvelAvg, baccelAvg;
//float scoreAvg;
//float bscoreAvg;

//TX Variables
//char adcString[10], ratioString[10], posString[10], txString[40]; //Should be faster.
//char scoreString[10]

//Timing variables
unsigned long startMillis;
unsigned long currentMillis;
unsigned long ledMillis;
unsigned long BLEMillis;

//Make sure these divide without remainders for best results
const int ledRate = 50; // LED flash rate (ms). Can go as fast as 10ms for better heartrate visibility.
const int sampleRate = 1.5; // ADC read rate (ms). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms
const int samplesPerRatio = 5; // Minimum number of samples per LED to accumulate before making a measurement. Adjust this with your LED rate so you sample across the whole flash at minimum.
const int BTRate = 100; // Bluetooth notify rate (ms). Min rate should be 10ms, however it will hang up if the buffer is not flushing. 100ms is stable.

void startADS() {
  // Begin ADS
  ads.begin();
  ads.setGain(GAIN_SIXTEEN);

  //ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV (default)
  //ads.setGain(GAIN_ONE);     // 1x gain   +/- 4.096V  1 bit = 2mV
  //ads.setGain(GAIN_TWO);     // 2x gain   +/- 2.048V  1 bit = 1mV
  //ads.setGain(GAIN_FOUR);    // 4x gain   +/- 1.024V  1 bit = 0.5mV
  //ads.setGain(GAIN_EIGHT);   // 8x gain   +/- 0.512V  1 bit = 0.25mV
  //ads.setGain(GAIN_SIXTEEN); // 16x gain  +/- 0.256V  1 bit = 0.125mV
  //Start timers
  startMillis = millis();
  ledMillis = millis();
}

void commandESP32(char received){
  if(received == 't'){
    sensorEnabled = true;
    digitalWrite(LED,LOW);
  }
  if(received == 'f'){
    sensorEnabled = false;
    digitalWrite(LED,HIGH); 
    digitalWrite(RED,LOW);
    digitalWrite(IR,LOW); 
    reset = true;
  }
  if(received == 'r'){
    reset = true;
  }
  if(received == 'p'){
    pIR_MODE = true;
    digitalWrite(RED,LOW);
    reset = true;
  }
  if(received == 'u'){
    if(USE_USB == false){
      USE_USB = true;
      Serial.begin(115200);
    }
    else { USE_USB = false; } // Serial.end()?
  }
  if(received == 'b') {
    if(USE_BLUETOOTH == false){
      USE_BLUETOOTH = true;
      SerialBT.begin("My_HEG");
    }
    else { USE_BLUETOOTH = false; }
  }
  delay(2000);
}

void setup() {
  if(USE_USB == true) {
    Serial.begin(115200);
  }
  pinMode(IR, OUTPUT);
  pinMode(RED, OUTPUT);
  //LOLIN32 ONLY
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);

  //esp_bt_sleep_disable(); // Disables sleep mode (debugging)
  SerialBT.begin("My_HEG");
  BLEMillis = millis();
}

void core_program() {
  if(sensorEnabled == true) {
    if(adcEnabled == false) {
      startADS();
      adcEnabled = true;
    }
    if(SEND_DUMMY_VALUE != true) {    
      if(currentMillis - startMillis >= sampleRate) {
        // read the analog in value:
        adc0 = ads.readADC_SingleEnded(0);
        //Voltage = (adc0 * bits2mv);
        // print the results to the Serial Monitor:
        if (VIEW_ADC_VALUE == true) {
          //Serial.println("ADC Value: ");
          //Serial.println(adc0);
          //Serial.println("\tVoltage: "); 
          //Serial.println(Voltage,7);
        }
        if (DEBUG_ADC == false) {
          if((adc0 >= 3000) || (reset == true)) { // The gain is high but anything over 7000 is most likely not a valid signal, anything more than 2000 is not likely your body's signal.
            //Serial.println("\nBad Read ");
            badSignal = true;
      
            //Temp: reset baseline on bad read
            signalDetermined = false;
            reset = false;
            baseline = 0;
      
            ticks0 = 0; // Reset counter
            ticks1 = 0;
            ticks2 = 0;
            redValue = 0; // Reset values
            irValue = 0;
            redAvg = 0;
            irAvg = 0;
            ratioAvg = 0;
            posAvg = 0;
          }
          else {
            if(badSignal == true){
              badSignal = false;
            }
            if(signalDetermined == false){
              ticks0++;
              if(ticks0 > 500) { // Wait for 500 samples of good signal before getting baseline
                // IR IN 12, RED IN 13
                if((ticks1 < 500) && (ticks2 < 500)) { // Accumulate samples for baseline
                  if(red_led == true) { // RED
                    redValue += adc0;
                    ticks1++;
                  }
                  else { // IR
                    irValue += adc0;
                    ticks2++;
                  }
                  //Serial.println("\nGetting Baseline. . .");
                }
                else {
                  signalDetermined = true;
                  redAvg = redValue * 100 / ticks1;
                  irAvg = irValue / ticks2;
      
                  baseline = redAvg / irAvg; // Set baseline ratio
                  ratio = baseline;
                  ticks0 = 0; // Reset counters
                  ticks1 = 0;
                  ticks2 = 0;
                  ticks3++;
                  redValue = 0; // Reset values
                  irValue = 0;
                  
                  //Uncomment this
                  //Serial.println("\tBaseline R: ");
                  //Serial.print(baseline,4);
                }
              }
            }
            else {
              ticks0++;
              if(red_led == true) { // RED
                redValue += adc0;
                ticks1++;
              }
              else { // IR
                irValue += adc0;
                ticks2++;
              }
              if((ticks2 > samplesPerRatio) && (ticks1 > samplesPerRatio)) { // Accumulate 50 samples per LED before taking reading
                redAvg = redValue * 100 / ticks1; // Divide value by number of samples accumulated // Scalar multiplier to make changes more apparent
                irAvg = irValue / ticks2;
                ratio = redAvg / irAvg; // Get ratio
                ratioAvg += ratio;
                bratioAvg += ratio;
                
                p1 = p2;
                p2 = ratio - baseline; // Position
                posAvg += p2 - p1;
                bposAvg += p2;
                //v1 = v2;
                //v2 = (p2 - p1) * ticks0 * 0.001; // Velocity in ms
                //velAvg += v2;
                
                //accel = (v2 - v1) * ticks0 * 0.001; // Acceleration in ms^2
                //accelAvg += accel;
                
                //score += ratio-baseline; // Simple scoring method. Better is to compare current and last SMA
                //scoreAvg += score;
                
                /*
                Serial.print("\tBaseline R: ");
                Serial.print(baseline,4);
                //Serial.print("\tRed: ");
                //Serial.print(redAvg);
                //Serial.print("\tIR: ");
                //Serial.print(irAvg);
                Serial.print("\tCurrent R: ");
                Serial.print(ratio,4);
                Serial.print("\trPosition: ");
                Serial.print(p2,4);
                Serial.print("\trVelocity: ");
                Serial.print(v2,4);
                Serial.print("\trAcceleration: ");
                Serial.print(accel,4);
                Serial.print("\trScore: ");
                Serial.print(score);
                Serial.print("\n");
                */
                ticks0 = 0; //Reset Counters
                ticks1 = 0;
                ticks2 = 0;

                ticks3++;
                bticks3++;
                
                redValue = 0; //Reset values to get next average
                irValue = 0;
              }
            }
          }
          
          startMillis = currentMillis;
        }
      }
    
      // Switch LEDs back and forth.
      // PUT IR IN 13, AND RED IN 12
      if(currentMillis - ledMillis >= ledRate) {
        if(red_led == false) {
          if(pIR_MODE == false){ // no LEDs in pIR mode, just raw IR from body heat emission.
            digitalWrite(RED,HIGH);
            digitalWrite(IR,LOW);
          }
          red_led = true;
        }
        else {
          if(pIR_MODE == false) {
            digitalWrite(RED,LOW);
            digitalWrite(IR,HIGH);
          }
          red_led = false;
        }
        ledMillis = currentMillis;
      }
      
      adcAvg += adc0;
      ticks4++;

      badcAvg += adc0;
      bticks4++;
      
    }
  }
 //DEBUG
 if(DEBUG == true){
  if(USE_USB == true){
    Serial.println("Heap after core_program cycle: ");
    Serial.println(ESP.getFreeHeap());
  }
  else {
    SerialBT.println("Heap after core_program cycle: ");
    SerialBT.println(ESP.getFreeHeap());
  }
 }
}

void bluetooth() {
  if(currentMillis - BLEMillis >= BTRate) { //SerialBT bitrate: ?/s. 100ms works, 50ms does cause hangups (which the LED flashes will reflect) - use smarter buffering.
      SerialBT.flush();
      if(SEND_DUMMY_VALUE != true) {
        if(bticks4 > 0) {
          if(USE_USB == false){
            badcAvg = badcAvg / ticks4;
          }
          /*
          memset(txString,0,sizeof(txString));
          dtostrf(adcAvg, 1, 0, adcString); 
          strcpy(txString,adcString);
          */
          if(bticks3 > 0) {
            if(USE_USB == false) {
              bratioAvg = bratioAvg / ticks3;
              bposAvg = bposAvg / ticks3;
              //bvelAvg = bvelAvg / ticks3;
              //baccelAvg = baccelAvg / tick3;
            
              //bscoreAvg = bscoreAvg / ticks3;
            }
    
            /*
            dtostrf(bratioAvg, 1, 5, ratioString);
            dtostrf(bposAvg, 1, 5, posString);
            
            strcat(txString,",");
            strcat(txString,ratioString);
            strcat(txString,",");
            strcat(txString,posString);
            strcat(txString,"\r\n");
            
            SerialBT.print(txString); // Should be faster.
            */
            //Using String is ill-advised yet this works better than separate print calls as far as I can tell. Above strcat method should be faster.
            SerialBT.print(String(badcAvg,0) + "," + String(bratioAvg,4) + "," + String(bposAvg,4)+"\r\n"); 
          }
          else {
            //strcat(txString,",WAIT\r\n");
            //SerialBT.print(txString);
            SerialBT.print(String(badcAvg,0) + ",WAIT\r\n");
          }
          
          //String strToSend = "{ratio:"+String(ratioAvg)+",adc:"+String(adc)+"}";
          //SerialBT.write((uint8_t*) buffer, strToSend*sizeof(int32_t)); //Faster to use a binary buffer
          //SerialBT.println(count);
          //count++;
          
          bratioAvg = 0;
          bposAvg = 0;
          badcAvg = 0;
          adc0 = 0;
          bticks3 = 0;
          bticks4 = 0;
          
          BLEMillis = currentMillis;
        }
      }
      else { 
        SerialBT.print("DUMMY,"+String(random(0,100))+","+String(random(0,100))+"\r\n");   
        }
      delay(10);
  }
  if(DEBUG == true){
     if(USE_USB == true){
      Serial.println("Heap after core_program cycle: ");
      Serial.println(ESP.getFreeHeap());
    }
    else {
      SerialBT.println("Heap after core_program cycle: ");
      SerialBT.println(ESP.getFreeHeap());
    }
  }
}

void usbSerial(){
  Serial.flush();
  if(SEND_DUMMY_VALUE != true) {
      if(ticks4 > 0) {
        adcAvg = adcAvg / ticks4;
        /*
        memset(txString,0,sizeof(txString));
        dtostrf(adcAvg, 1, 0, adcString); 
        strcpy(txString,adcString);
        */
        if(ticks3 > 0) {
          ratioAvg = ratioAvg / ticks3;
          posAvg = posAvg / ticks3;
          
          //velAvg = velAvg / ticks3;
          //accelAvg = accelAvg / tick3;
          
          //scoreAvg = scoreAvg / ticks3;
          /*
  
          dtostrf(ratioAvg, 1, 5, ratioString);
          dtostrf(posAvg, 1, 5, posString);
          
          strcat(txString,",");
          strcat(txString,ratioString);
          strcat(txString,",");
          strcat(txString,posString);
          strcat(txString,"\r\n");
          
          SerialBT.print(txString); // Should be faster.
          */
          Serial.print(adcAvg,0);
          Serial.print(',');
          Serial.print(ratioAvg,4);
          Serial.print(',');
          Serial.println(posAvg,4);
        }
        else {
          //strcat(txString,",WAIT\r\n");
          //Serial.print(txString);
          Serial.print(adcAvg,0);
          Serial.println(",WAIT");
        }
        
        //String strToSend = "{ratio:"+String(ratioAvg)+",adc:"+String(adc)+"}";
        //Serial.write((uint8_t*) buffer, strToSend*sizeof(int32_t)); //Faster to use a binary buffer
        //Serial.println(count);
        //count++;
        
        ratioAvg = 0;
        posAvg = 0;
        adcAvg = 0;
        adc0 = 0;
        ticks3 = 0;
        ticks4 = 0;
        
      }
    }
    else { 
      Serial.print("DUMMY,"+String(random(0,100))+","+String(random(0,100))+"\r\n");   
      }
  if(DEBUG == true){
     if(USE_USB == true){
      Serial.println("Heap after core_program cycle: ");
      Serial.println(ESP.getFreeHeap());
    }
    else {
      SerialBT.println("Heap after core_program cycle: ");
      SerialBT.println(ESP.getFreeHeap());
    }
  }
}

void checkInput(){
  if(USE_BLUETOOTH == true) {
    if(SerialBT.available()){
      received = SerialBT.read();
      SerialBT.println(received);
      commandESP32(received);
    }
  }
  if(USE_USB == true) {
    if(Serial.available()){
      received = Serial.read();
      Serial.println(received);
      commandESP32(received);
    }
  }
}

void loop() {
  currentMillis = millis();
  checkInput();
  core_program();
  if(USE_USB == true){
    usbSerial();
  }
  if(USE_BLUETOOTH == true){
    if(SerialBT.hasClient() == true) {
      bluetooth();
    }
  }
  delay(1);
}

