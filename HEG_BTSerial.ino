// Joshua Brewster - HEG BTSerial
// Requires Arduino ADS1x15 library and Arduino ESP32 library, as well as a compatible ESP32 board

// TEST 0.93
// 3/29/2019
/*
   TODO
   - HRV basic calculation, adjust LED flash rate accordingly (50ms is viable) Alternatively, use MAX30102 on 2nd i2c?
   - Accurate SpO2 reading? Can re-appropriate examples from Sparkfun MAX30105 library
   - Optimize memory usage. Take better advantage of ESP32.
   - Switch to hardware ISR timer. esp_timer_get_time() in microseconds.
   - More data modes, transmitter modes (e.g. wifi vs bluetooth) might have to stay in separate sketches to preserve esp32 memory. The 16MB ESP32 might not have any trouble there.
*/

#include "BluetoothSerial.h"

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <esp_timer.h>
//#include <esp_bt.h>
//#include <ArduinoJson.h>


BluetoothSerial SerialBT;

bool USE_USB = true; // WRITE 'u' TO TOGGLE, CHANGE HERE TO SET DEFAULT ON POWERING THE DEVICE
bool USE_BLUETOOTH = true; // WRITE 'b' TO TOGGLE. 
bool pIR_MODE = false; // SET TO TRUE OR WRITE 'p' TO DO PASSIVE INFRARED ONLY (NO RED LIGHT FOR BLOOD-OXYGEN DETECTION). RATIO IS USELESS HERE, USE ADC CHANGES AS MEASUREMENT.

bool DEBUG_ESP32 = false;
bool DEBUG_ADC = false; // FOR USE IN ARDUINO IDE
bool DEBUG_LEDS = false;
bool SEND_DUMMY_VALUE = false;

//Make sure ledRate and sampleRate divide without remainders for best results
const int8_t ledRate = 25; // LED flash rate (ms). Can go as fast as 10ms for better heartrate visibility.
const int8_t sampleRate = 1.5; // ADC read rate (ms). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms. Bluetooth limits it.
const int8_t samplesPerRatio = 3; // Minimum number of samples per LED to accumulate before making a measurement. Adjust this with your LED rate so you sample across the whole flash at minimum.
const int8_t BTRate = 100; // Bluetooth notify rate (ms). Min rate should be 10ms, however it will hang up if the buffer is not flushing. 100ms is stable.
const int8_t USBRate = 0; // No need to delay USB unless on old setups.

// PUT IR IN 13, RED IN 12
int IR = 13;
int RED = 12;

//int IR2 = 33;
//int RED2 = 32;

const int LED = 5; // Lolin32 V1.0.0 LED on Pin 5
const int PWR = 14; // Powers ADC and OPT101

//SET NON-DEFAULT SDA AND SCL PINS
#define SDA0_PIN 16
#define SCL0_PIN 4

//#define SDA1_PIN ?
//#define SCL1_PIN ?

// HEG VARIABLES
int count = 0;
bool sensorEnabled = false;
bool adcEnabled = false;
bool reset = false;

char received;

int16_t adc0; // Resulting 15 bit integer.

//Setup ADS1115
Adafruit_ADS1115 ads(0x48);
long adcChannel = 0; //Channel on the ADC to read. Default 0.

float Voltage = 0.0;
float range = 32767; // 16 bit ADC (15 bits of range minus one)
float gain = 0.256; // +/- V
float bits2mv = gain / range;

//Signal flags
bool red_led = false; // Bools to alternate LEDS
bool ir_led = false;
bool no_led = true; // Both LEDS begin off so default is true
bool badSignal = false; // Bool for too high of an ADC reading
bool signalDetermined = false; // Bool for whether the ADC reading is within desired range

//Counters
int ticks0, redTicks, irTicks, ratioTicks, adcTicks, noLEDTicks = 0;
int bratioTicks, badcTicks = 0; //BT signal on separate counters 

//Scoring variables
long redValue = 0;
long irValue = 0;
long rawValue = 0;
float redAvg = 0;
float irAvg = 0;
float rawAvg = 0;
float ratio = 0;
float baseline = 0;

float p1, p2 = 0;
float v1, v2 = 0;
float accel = 0;

float ratioAvg, adcAvg, posAvg; //velAvg, accelAvg;
float bratioAvg, badcAvg, bposAvg; //bvelAvg, baccelAvg;

//TX Variables
//char adcString[10], ratioString[10], posString[10], txString[40]; // Faster in BLE mode
//char scoreString[10]

//Scoring variables
/*
int smaTicks;
float slowSMA[20], lastSlow[20];
float fastSMA[5], lastFast[5];
float score = 0;
float scoreAvg;
float bscoreAvg;

void score(){
  slowSMA += ratio;
  smaTicks++;
  if(smaTicks >= 15) {
    fastSMA += ratio;
  }
  if(smaTicks >= 20){
    slowSMA = slowSMA/20;
    fastSMA = fastSMA/5;
    score = fastSMA - slowSMA;
    scoreAvg += score;
  }
}
*/

//Timing variables
unsigned long sampleMillis;
unsigned long currentMillis;
unsigned long ledMillis;
unsigned long BLEMillis;
unsigned long USBMillis;


//Start ADC and set gain. Starts timers
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
  
  adcEnabled = true;
}

//Character commands, may use Strings too, BLE likes chars more.
void commandESP32(char received) {
  if (received == 't') { //Enable Sensor
    sensorEnabled = true;
    digitalWrite(LED, LOW);
  }
  if (received == 'f') {  //Disable sensor, reset.
    sensorEnabled = false;
    digitalWrite(LED, HIGH);
    digitalWrite(RED, LOW);
    digitalWrite(IR, LOW);
    reset = true;
  }
  if (received == 'r') {  //Reset baseline and readings
    reset = true;
  }
  if (received == 'p') {  //pIR Toggle
    pIR_MODE = true;
    digitalWrite(RED, LOW);
    reset = true;
  }
  if (received == 'u') {  //USB Toggle
    if (USE_USB == false) {
      USE_USB = true;
      Serial.begin(115200);
    }
    else {
      USE_USB = false;  // Serial.end() or Serial.hasClient() type function to suppress serial output? 
    }
  }
  if (received == 'b') {  //Bluetooth Toggle
    if (USE_BLUETOOTH == false) {
      USE_BLUETOOTH = true;
      SerialBT.begin("My_HEG");
    }
    else {
      USE_BLUETOOTH = false;
    }
  }
  if ((received == '0') || (received == '1') || (received == '2') || (received == '3')){
    adcChannel = received - '0';
    reset = true;
  }
  if (received == 'd'){ // Dual Sensor toggle, changes LED pinouts and adcChannel.
    digitalWrite(RED,LOW);
    digitalWrite(IR,LOW);
    if(adcChannel == 0){
      IR = 33;
      RED = 32;
      adcChannel = 1;
      pinMode(IR,OUTPUT);
      pinMode(RED,OUTPUT);
    }
    else {
      IR = 12;
      RED = 13;
      adcChannel = 0;
    }
    reset = true;
  }
  delay(2000);
}

void setup() {
  Wire.begin(SDA0_PIN,SCL0_PIN); //Use in case of non-default SDA/SCL pins
  //Wire1.begin(SDA1_PIN,SCL1_PIN);
  if (USE_USB == true) {
    Serial.begin(115200);
  }
  pinMode(IR, OUTPUT);
  pinMode(RED, OUTPUT);
  
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);
  
  //LOLIN32 ONLY
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  //esp_bt_sleep_disable(); // Disables sleep mode (debugging)
  SerialBT.begin("My_HEG");
  BLEMillis = millis();
  USBMillis = millis();
}

// Core loop for HEG sampling.
void core_program() {
  if (sensorEnabled == true) {
    if (adcEnabled == false) {
      startADS();
      //Start timers
      sampleMillis = millis();
      ledMillis = millis();
    }
    if (SEND_DUMMY_VALUE != true) {
      // Switch LEDs back and forth.
      // PUT IR IN 13, AND RED IN 12
      if (currentMillis - ledMillis >= ledRate) {
        if (red_led == true) { // IR on
          if (pIR_MODE == false) {
            digitalWrite(RED, LOW);
            digitalWrite(IR, HIGH);
            if(DEBUG_LEDS == true) {
              Serial.println("IR ON");
            }
          }
          red_led = false;
          ir_led = true;
          no_led = false;
        }
        else if ((red_led == false)){// && (no_led == true)) { // Red on
          if (pIR_MODE == false) { // no LEDs in pIR mode, just raw IR from body heat emission.
            digitalWrite(RED, HIGH);
            digitalWrite(IR, LOW);
            if(DEBUG_LEDS == true){
              Serial.println("RED ON");
            }
          }
          red_led = true;
          ir_led = false;
          no_led = false;
        }
        else { // No LEDs
          digitalWrite(RED,LOW);
          digitalWrite(IR,LOW);
          if(DEBUG_LEDS == true){
            Serial.println("NO LED");
          }
          red_led = false;
          ir_led = false;
          no_led = true;
        }
        ledMillis = currentMillis;
      }
      
      if (currentMillis - sampleMillis >= sampleRate) {
        // read the analog in value:
        adc0 = ads.readADC_SingleEnded(adcChannel); // -1 indicates something wrong with the ADC
        //Voltage = (adc0 * bits2mv);
      
        // print the results to the Serial Monitor:
        if (DEBUG_ADC == true) {
          Serial.println("ADC Value: ");
          Serial.println(adc0);
          //Serial.println("\tVoltage: ");
          //Serial.println(Voltage,7);
        }
        else {
          if ((adc0 >= 6000) || (reset == true)) { // The gain is high but anything over 3000 is most likely not a valid signal, anything more than 2000 is not likely your body's signal.
            //Serial.println("\nBad Read ");
            badSignal = true;

            //Temp: reset baseline on bad read
            signalDetermined = false;
            reset = false;
            baseline = 0;

            ticks0 = 0; // Reset counter
            redTicks = 0;
            irTicks = 0;
            noLEDTicks = 0;
            redValue = 0; // Reset values
            irValue = 0;
            rawValue = 0;
            redAvg = 0;
            irAvg = 0;
            rawAvg = 0;
            ratioAvg = 0;
            posAvg = 0;
          }
          else {
            if (badSignal == true) {
              badSignal = false;
            }
            if (signalDetermined == false) { // GET BASELINE
              ticks0++;
              if (ticks0 > 100) { // Wait n samples of good signal before getting baseline
                // IR IN 12, RED IN 13
                if ((redTicks < 30) && (irTicks < 30)){// && (noLEDTicks < 250)) { // Accumulate samples for baseline
                  if (red_led == true) { // RED
                    redValue += adc0;
                    redTicks++;
                  }
                  else if (ir_led == true) { // IR
                    irValue += adc0;
                    irTicks++;
                  }
                  else {
                    rawValue += adc0;
                    noLEDTicks++;
                  }
                  //Serial.println("\nGetting Baseline. . .");
                }
                else {
                  signalDetermined = true;
                  
                  //rawAvg = rawValue / noLEDTicks;
                  redAvg = log10((redValue / redTicks));// - rawAvg);
                  irAvg = log10((irValue / irTicks));// - rawAvg);

                  baseline = (redAvg / irAvg) * 100; // Set baseline ratio, multiply by 100 for scaling.
                  ratioAvg += baseline; // First ratio sent via serial will be baseline.
                  bratioAvg += ratioAvg;

                  redValue = 0; // Reset values
                  irValue = 0;
                  rawValue = 0;
                  ticks0 = 0; // Reset counters
                  redTicks = 0;
                  irTicks = 0;
                  noLEDTicks = 0;
                  ratioTicks++;
                  bratioTicks++;
                  
                  //Uncomment this for baseline printing
                  //Serial.println("\tBaseline R: ");
                  //Serial.print(baseline,4);
                }
              }
            }
            else { // GET RATIO
              ticks0++;
              if (red_led == true) { // RED
                redValue += adc0;
                redTicks++;
              }
              else if (ir_led == true) { // IR
                irValue += adc0;
                irTicks++;
              }
              else {
                rawValue += adc0;
                noLEDTicks++;
              }
              if ((redTicks >= samplesPerRatio) && (irTicks >= samplesPerRatio)){// && (noLEDTicks >= samplesPerRatio)) { // Accumulate 50 samples per LED before taking reading
                //rawAvg = rawValue / noLEDTicks;
                redAvg = log10((redValue / redTicks));// - rawAvg); // Divide value by number of samples accumulated // Scalar multiplier to make changes more apparent
                irAvg = log10((irValue / irTicks));// - rawAvg);
                ratio = (redAvg / irAvg) * 100; // Get ratio, multiply by 100 for scaling.
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
                redTicks = 0;
                irTicks = 0;
                noLEDTicks = 0;

                ratioTicks++;
                bratioTicks++;

                redValue = 0; //Reset values to get next average
                irValue = 0;
                rawValue = 0;
              }
            }
          }
        }
        sampleMillis = currentMillis;
      }

      adcAvg += adc0;
      adcTicks++;

      badcAvg += adc0;
      badcTicks++;
    }
  }

  //DEBUG_ESP32
  if (DEBUG_ESP32 == true) {
    if (USE_USB == true) {
      Serial.println("Heap after core_program cycle: ");
      Serial.println(ESP.getFreeHeap());
    }
    else {
      SerialBT.println("Heap after core_program cycle: ");
      SerialBT.println(ESP.getFreeHeap());
    }
  }
} // END core_program()

// Send Bluetooth serial data
void bluetooth() {
  if (currentMillis - BLEMillis >= BTRate) { //SerialBT bitrate: ?/s. 100ms works, 50ms does cause hangups (which the LED flashes will reflect) - use smarter buffering.
    SerialBT.flush();
    if (SEND_DUMMY_VALUE != true) {
      if (badcTicks > 0) {
          badcAvg = badcAvg / badcTicks;
        /*
          memset(txString,0,sizeof(txString));
          dtostrf(adcAvg, 1, 0, adcString);
          strcpy(txString,adcString);
        */
        if (bratioTicks > 0) {
            bratioAvg = bratioAvg / bratioTicks;
            bposAvg = bposAvg / bratioTicks;
            //bvelAvg = bvelAvg / bratioTicks;
            //baccelAvg = baccelAvg / bratioTicks;

            //bscoreAvg = bscoreAvg / bratioTicks;
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
          SerialBT.print(String(badcAvg, 0) + "," + String(bratioAvg, 4) + "," + String(bposAvg, 4) + "\r\n");
        }
        else {
          //strcat(txString,",WAIT\r\n");
          //SerialBT.print(txString);
          SerialBT.print(String(badcAvg, 0) + ",WAIT\r\n");
        }

        //String strToSend = "{ratio:"+String(ratioAvg)+",adc:"+String(adc)+"}";
        //SerialBT.write((uint8_t*) buffer, strToSend*sizeof(int32_t)); //Faster to use a binary buffer
        //SerialBT.println(count);
        //count++;
        rawAvg = 0;
        bratioAvg = 0;
        bposAvg = 0;
        badcAvg = 0;
        adc0 = 0;
        bratioTicks = 0;
        badcTicks = 0;

      }
    }
    else {
      SerialBT.print("DUMMY," + String(random(0, 100)) + "," + String(random(0, 100)) + "\r\n");
    }
  if (DEBUG_ESP32 == true) {
    if (USE_USB == true) {
      Serial.println("Heap after core_program cycle: ");
      Serial.println(ESP.getFreeHeap());
    }
    else {
      SerialBT.println("Heap after core_program cycle: ");
      SerialBT.println(ESP.getFreeHeap());
    }
  }
    BLEMillis = currentMillis;
    delay(10); // 10ms delay required due to bottleneck in library.
  }
}

// Send USB serial data.
void usbSerial() {
  Serial.flush();
  if(currentMillis - USBMillis >= USBRate) {
    if (SEND_DUMMY_VALUE != true) {
      if (adcTicks > 0) {
        adcAvg = adcAvg / adcTicks;
        /*
          memset(txString,0,sizeof(txString));
          dtostrf(adcAvg, 1, 0, adcString);
          strcpy(txString,adcString);
        */
        if (ratioTicks > 0) {
          ratioAvg = ratioAvg / ratioTicks;
          posAvg = posAvg / ratioTicks;

          //velAvg = velAvg / ratioTicks;
          //accelAvg = accelAvg / ratioTicks;

          //scoreAvg = scoreAvg / ratioTicks;
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
          Serial.print(adcAvg, 0);
          Serial.print(',');
          Serial.print(ratioAvg, 4);
          Serial.print(',');
          Serial.println(posAvg, 4);
        }
        else {
          //strcat(txString,",WAIT\r\n");
          //Serial.print(txString);
          Serial.print(adcAvg, 0);
          Serial.println(",WAIT");
        }

        //String strToSend = "{ratio:"+String(ratioAvg)+",adc:"+String(adc)+"}";
        //Serial.write((uint8_t*) buffer, strToSend*sizeof(int32_t)); //Faster to use a binary buffer
        //Serial.println(count);
        //count++;

        rawAvg = 0;
        ratioAvg = 0;
        posAvg = 0;
        adcAvg = 0;
        adc0 = 0;
        ratioTicks = 0;
        adcTicks = 0;
      }
    }
    else {
      Serial.print("DUMMY," + String(random(0, 100)) + "," + String(random(0, 100)) + "\r\n");
    }
    if (DEBUG_ESP32 == true) {
      if (USE_USB == true) {
        Serial.println("Heap after core_program cycle: ");
        Serial.println(ESP.getFreeHeap());
      }
      else {
        SerialBT.println("Heap after core_program cycle: ");
        SerialBT.println(ESP.getFreeHeap());
      }
    }
    USBMillis = currentMillis;
  }
}

void checkInput() {
  if (USE_BLUETOOTH == true) {
    if (SerialBT.available()) {
      received = SerialBT.read();
      SerialBT.println(received);
      commandESP32(received);
    }
  }
  if (USE_USB == true) {
    if (Serial.available()) {
      received = Serial.read();
      Serial.println(received);
      commandESP32(received);
    }
  }
}

void loop() {
  delay(1); // temp, figure out why getting rid of this blocks serial inputs
  currentMillis = millis();
  checkInput();
  core_program();
  if (USE_USB == true) {
    usbSerial();
  }
  if (USE_BLUETOOTH == true) {
    if (SerialBT.hasClient() == true) {
      bluetooth();
    }
  }
}




