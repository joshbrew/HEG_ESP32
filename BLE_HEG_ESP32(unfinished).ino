
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
//BLERemoteCharacteristic * pRemoteCharacteristic;
bool deviceConnected = true; // Set to true for debugging via USB
bool Notifying = false;

uint8_t txValue = 0;

// HEG VARIABLES
bool DEBUG = false;
bool VIEW_ADC_VALUE = false;
bool SEND_DUMMY_VALUE = false;

// PUT IR IN 13, RED IN 12
const int IR = 13;
const int RED = 12;
const int LED = 5; // Lolin32V1.0.0 LED on Pin 5
int16_t adc0; // Resulting 16 bit integer

//Setup ADS1115
Adafruit_ADS1115 ads;

float Voltage = 0.0;
float range = 32767; // 16 bit ADC (15 bits of range minus one)
float gain = 0.256; // +/- V
float bits2mv = gain / range;

//Signal flags
bool first_led = false; // Bool to alternate LEDS
bool badSignal = false;
bool signalDetermined = false;

//Counters
int ticks0, ticks1, ticks2, ticks3, ticks4 = 0;

//Scoring variables
long redValue = 0;
long irValue = 0;
float redavg = 0;
float iravg = 0;
float ratio = 0;
float baseline = 0;

float score = 0;

float p1, p2 = 0;
float v1, v2 = 0;
float accel = 0;

float ratioAvg;
float adcAvg;

//Timing variables
unsigned long startMillis;
unsigned long currentMillis;
unsigned long ledMillis;
unsigned long BLEMillis;

//Make sure these divide without remainders for best results
const unsigned long ledRate = 50; // LED flash rate (ms)
const unsigned long sampleRate = 2; // ADC read rate (ms). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "cf5990ef-6f81-4a15-9439-38b9c2d9bbdd" // UART service UUID
#define CHARACTERISTIC_UUID_RX "bc5fe0e0-44b3-4458-897a-f525b1e7927d"
#define CHARACTERISTIC_UUID_TX "475e4bad-df46-4cb7-b7f3-98de2682bc5e"

void StartADS() {
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

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device Connected");
      
      //LOLIN32 ONLY
      digitalWrite(LED, LOW);
      
      //StartADS();
      //BLEMillis = millis();
      Notifying = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      
      //LOLIN32 ONLY
      digitalWrite(LED, HIGH);
      
      digitalWrite(IR,LOW);
      digitalWrite(RED,LOW);
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      Notifying = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {

        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]); }

        
        Serial.println("*********");
      }

      /* 
       *  if (rxValue == "0x01") {
       *    Notifying = true;
       *  }
       *  if (rxValue == "0x00") {
       *    Notifying = false;
       *  }
       */
      
    }
};

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(IR, OUTPUT);
  pinMode(RED, OUTPUT);
  //LOLIN32 ONLY
  pinMode(LED, OUTPUT);
  
  //LOLIN32 ONLY
  digitalWrite(LED, HIGH);
  
  Serial.println("Starting BLE work!");
  
  BLEDevice::init("HEG");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());
  //pRemoteCharacteristic->registerForNotify(notifyCallback);
  
  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->addServiceUUID(pService->getUUID());
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  StartADS();
  BLEMillis = millis();

}

void loop() {

    if (deviceConnected) { // && Notifying) 
        currentMillis = millis();
        
        if(currentMillis - startMillis >= sampleRate) {
      
          // read the analog in value:
          adc0 = ads.readADC_SingleEnded(0);
          //Voltage = (adc0 * bits2mv);
          
          // print the results to the Serial Monitor:
          if (VIEW_ADC_VALUE == true) {
            Serial.println("ADC Value: ");
            Serial.println(adc0);
            //Serial.println("\tVoltage: "); 
            //Serial.println(Voltage,7);
          }
          if (DEBUG == false) {
            if(adc0 >= 7000) { // The gain is high but anything over 7000 is most likely not a valid signal
              Serial.println("\nBad Read ");
              badSignal = true;
        
              //Temp: reset baseline on bad read
              signalDetermined = false;
              baseline = 0;
        
              ticks0 = 0; // Reset counter
              ticks1 = 0;
              ticks2 = 0;
              redValue = 0; // Reset values
              irValue = 0;
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
                    Serial.println("\nGetting Baseline. . .");
                  }
                  else {
                    signalDetermined = true;
                    redavg = redValue / ticks1;
                    iravg = irValue / ticks2;
        
                    baseline = redavg / iravg; // Set baseline ratio
                    ticks0 = 0; // Reset counters
                    ticks1 = 0;
                    ticks2 = 0;
                    redValue = 0; // Reset values
                    irValue = 0;
                    
                    //Uncomment this
                    Serial.println("\tBaseline R: ");
                    Serial.print(baseline,4);
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
                if((ticks2 > 75) && (ticks1 > 75)) { // Accumulate 50 samples per LED before taking reading
                  redavg = redValue / ticks1; // Divide value by number of samples accumulated
                  iravg = irValue / ticks2;
                  ratio = redavg / iravg; // Get ratio
                  ratioAvg += ratio;
                 
                  p1 = p2;
                  p2 = ratio - baseline; // Position
                  v1 = v2;
                  v2 = (p2 - p1) * ticks0 * 0.001; // Velocity in ms
                  accel = (v2 - v1) * ticks0 * 0.001; // Acceleration in ms^2
                  score += ratio-baseline; // Simple scoring method

                  Serial.print("\tBaseline R: ");
                  Serial.print(baseline,4);
                  //Serial.print("\tRed: ");
                  //Serial.print(redavg);
                  //Serial.print("\tIR: ");
                  //Serial.print(iravg);
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

        if(Notifying == true) {
          if(currentMillis - BLEMillis >= 50) { //Delay 10ms minimum to not overwhelm bluetooth packets - MAX THROUGHPUT 90KB/s
           if(SEND_DUMMY_VALUE == false) {
            
            // Let's convert the values to a char array:
            char txString1[14], txString2[14]; // make sure these are big enough
  
            if(ticks3 > 0) {
              ratioAvg = ratioAvg / ticks3;
              dtostrf(ratioAvg, 1, 3, txString1); // float_val, min_width, digits_after_decimal, char_buffer
            }
            else {
              strcpy(txString1,"GETTING_RATIO");
            }
            
            adcAvg = adcAvg / ticks4;
            dtostrf(adcAvg, 1, 3, txString2);
            
            char txString[30];
            strcpy(txString,txString1);
            strcat(txString,",");
            strcat(txString,txString2);
            
            //    pTxCharacteristic->setValue(&adcAvg, 1); // To send the integer value
            //    pTxCharacteristic->setValue("Hello!"); // Sending a test message
            pTxCharacteristic->setValue(txString);
            pTxCharacteristic->notify(); // Send the value to the app!
            //Serial.print("*** Sent Value: ");
            //Serial.print(txString1);
            //Serial.println(" ***");
            //Serial.print(txString2);
            //Serial.println(" ***");
            //Serial.println(BLEMillis);
    
            ratioAvg = 0;
            adcAvg = 0;
            adc0 = 0;
            ticks3 = 0;
            ticks4 = 0;
              }
           else {
            //Dummy values
            char txString1[10];
            char txString2[10];
            
            float rando = random(95,105) * 0.01;
            float rando2 = random(2000,3000);
            
            dtostrf(rando,1,3,txString1);
            dtostrf(rando2,1,3,txString2);
            
            char txString[30];
            strcpy(txString,"DUMMY:");
            strcat(txString,txString1);
            strcat(txString,",");
            strcat(txString,txString2);
            
            pTxCharacteristic->setValue(txString);
            pTxCharacteristic->notify(); // Send the value to the app!
            //Serial.print("*** Sent Value: ");
            //Serial.print(txString);
            //Serial.println(" ***");
              }
           BLEMillis = currentMillis;     
        }
      }
   }
      //End loop
}
