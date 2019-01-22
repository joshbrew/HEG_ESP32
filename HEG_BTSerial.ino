// Joshua Brewster - HEG BTSerial
/*
 * TODO
 * - HRV basic calculation, adjust LED flash rate accordingly (50ms is viable)
 * - Optimize memory usage. Take better advantage of ESP32.
 * - More data modes, transmitter modes might have to stay in separate sketches to preserve esp32 memory. The 16MB ESp32 might not have any trouble there.
 * 
 */

#include "BluetoothSerial.h"

#include <Wire.h>
#include <Adafruit_ADS1015.h>
//#include <esp_bt.h>
//#include <ArduinoJson.h>
 
BluetoothSerial SerialBT;

int count = 0;
bool sensorEnabled = false;
bool adcEnabled = false;
bool usbSerialEnabled = false;

char received;

// HEG VARIABLES
bool DEBUG = false;
bool DEBUG_ADC = false;
bool VIEW_ADC_VALUE = false;
bool SEND_DUMMY_VALUE = false;

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
bool first_led = false; // Bool to alternate LEDS
bool badSignal = false; // Bool for too high of an ADC reading
bool signalDetermined = false; // Bool for whether the ADC reading is within desired range

//Counters
int ticks0, ticks1, ticks2, ticks3, ticks4 = 0;

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
//float scoreAvg;

//TX Variables
//char adcString[10], ratioString[10], posString[10], txString[40]; //Should be faster.
//char scoreString[10]

//Timing variables
unsigned long startMillis;
unsigned long currentMillis;
unsigned long ledMillis;
unsigned long BLEMillis;

//Make sure these divide without remainders for best results
const unsigned long ledRate = 50; // LED flash rate (ms). Can go as fast as 10ms for better heartrate visibility.
const unsigned long sampleRate = 2; // ADC read rate (ms). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms
const unsigned long samplesPerRatio = 50; // Number of samples per LED to accumulate before making a measurement. Time = 2*samples*sampleRate
const unsigned long BTRate = 333; // Bluetooth notify rate (ms). Min rate should be 10ms, however it will hang up if the buffer is not flushing. 100ms is stable.

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
  }
  /*
  if(received == 'u'){
    usbSerialEnabled = true;
  }
  if(received == 'b') {
    usbSerialEnabled = false;
  }
  */
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  
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
      currentMillis = millis();
          
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
          if(adc0 >= 7000) { // The gain is high but anything over 7000 is most likely not a valid signal, anything more than 2000 is not likely your body's signal.
            //Serial.println("\nBad Read ");
            badSignal = true;
      
            //Temp: reset baseline on bad read
            signalDetermined = false;
            baseline = 0;
      
            ticks0 = 0; // Reset counter
            ticks1 = 0;
            ticks2 = 0;
            redValue = 0; // Reset values
            irValue = 0;
            ratioAvg = 0;
            posAvg = 0;
          }
          else {
            badSignal = false;
            if(signalDetermined == false){
              ticks0++;
              if(ticks0 > 500) { // Wait for 500 samples of good signal before getting baseline
                // IR IN 12, RED IN 13
                if((ticks1 < 500) && (ticks2 < 500)) { // Accumulate samples for baseline
                  if(first_led == true) { // RED
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
                  redAvg = redValue / ticks1;
                  irAvg = irValue / ticks2;
      
                  baseline = redAvg / irAvg; // Set baseline ratio
                  ticks0 = 0; // Reset counters
                  ticks1 = 0;
                  ticks2 = 0;
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
              if(first_led == true) { // RED
                redValue += adc0;
                ticks1++;
              }
              else { // IR
                irValue += adc0;
                ticks2++;
              }
              if((ticks2 > samplesPerRatio) && (ticks1 > samplesPerRatio)) { // Accumulate 50 samples per LED before taking reading
                redAvg = redValue / ticks1; // Divide value by number of samples accumulated
                //redAvg = redAvg * 200 // Scalar multiplier to make changes more apparent
                irAvg = irValue / ticks2;
                ratio = redAvg / irAvg; // Get ratio
                ratioAvg += ratio;
                
                p1 = p2;
                p2 = ratio - baseline; // Position
                posAvg += p2;
                
                //v1 = v2;
                //v2 = (p2 - p1) * ticks0 * 0.001; // Velocity in ms
                //velAvg += v2;
                
                //accel = (v2 - v1) * ticks0 * 0.001; // Acceleration in ms^2
                //accelAvg += accel;
                
                //score += ratio-baseline; // Simple scoring method
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
        if(first_led == false) {
          digitalWrite(RED,HIGH);
          digitalWrite(IR,LOW);
          first_led = true;
        }
        else {
          digitalWrite(RED,LOW);
          digitalWrite(IR,HIGH);
          first_led = false;
        }
        ledMillis = currentMillis;
      }
      
      adcAvg += adc0;
      ticks4++;
      
    }
  }
 //DEBUG
 if(DEBUG == true){
  Serial.println("Heap after core_program cycle: ");
  Serial.println(ESP.getFreeHeap());
 }
}

void bluetooth() {
  if(SerialBT.available()){
    received = SerialBT.read();
    Serial.println(received);
    SerialBT.println(received);
    commandESP32(received);
  }
  
  if(currentMillis - BLEMillis >= BTRate) { //SerialBT bitrate: ?/s. 100ms works, 50ms does cause hangups (which the LED flashes will reflect) - use smarter buffering.
      SerialBT.flush();
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
            //Using String is ill-advised yet this works better than separate print calls as far as I can tell. Above strcat method should be faster.
            SerialBT.print(String(adcAvg,0) + "," + String(ratioAvg,4) + "," + String(posAvg,4)+"\r\n"); 
          }
          else {
            //strcat(txString,",WAIT\r\n");
            //SerialBT.print(txString);
            SerialBT.print(String(adcAvg,0) + ",WAIT\r\n");
          }
          
          //String strToSend = "{ratio:"+String(ratioAvg)+",adc:"+String(adc)+"}";
          //SerialBT.write((uint8_t*) buffer, strToSend*sizeof(int32_t)); //Faster to use a binary buffer
          //SerialBT.println(count);
          //count++;
          
          ratioAvg = 0;
          posAvg = 0;
          adcAvg = 0;
          adc0 = 0;
          ticks3 = 0;
          ticks4 = 0;
          
          BLEMillis = currentMillis;
        }
      }
      else { 
        int rando = random(0,1000);
        SerialBT.print("DUMMY,"+String(rando)+"\r\n");   
        }
      delay(10);
  }
  if(DEBUG == true){
    Serial.println("Heap after bluetooth() cycle: ");
    Serial.println(ESP.getFreeHeap());
  }
}

void loop() {
  if(SerialBT.hasClient() == true) {
    core_program();
    bluetooth();
    delay(1); //1 ms delay.
  }
}
