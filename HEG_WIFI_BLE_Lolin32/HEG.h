// Joshua Brewster - HEG Arduino Functions
// Contributors - Diego Schmaedech (StateChanger Team)
// Requires Arduino ADS1x15 library and Arduino ESP32 library, as well as a compatible ESP32 board

/*
   TODO
   - HRV basic calculation, adjust LED flash rate accordingly (50ms is viable at bare min) Alternatively, get an ECG chip or use the MAX30102?
   - Accurate SpO2 reading?
   - Check power draw  
   */

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <esp_timer.h>
//#include <esp_bt.h>
//#include <ArduinoJson.h>

#include "BLE_API.h"

//#include <SavLayFilter.h>
//SavLayFilter smallFilter(&filteredRatio, 0, 5);  //Cubic smoothing with windowsize of 5
//SavLayFilter largeFilter(&filteredRatio, 0, 25); //Cubic smoothing with windowsize of 25

bool USE_USB = true;          // WRITE 'u' TO TOGGLE, CHANGE HERE TO SET DEFAULT ON POWERING THE DEVICE
bool USE_BT = false;          // WRITE 'b' TO TOGGLE.
bool pIR_MODE = false;        // SET TO TRUE OR WRITE 'p' TO DO PASSIVE INFRARED ONLY (NO RED LIGHT FOR BLOOD-OXYGEN DETECTION). RATIO IS USELESS HERE, USE ADC CHANGES AS MEASUREMENT.
bool NOISE_REDUCTION = false; // WRITE 'n' TO TOGGLE USING 4 LEDS FOR NOISE CANCELLING *EXPERIMENTAL*
bool USE_DIFF = false;        // Use differential read mode, can reduce noise.
bool USE_2_3 = false;         // Use channels 2 and 3 for differential read.
bool USE_AMBIENT = true;     // Subtract the value of an intermediate no-led reading (good in case of voltage bleeding).
bool ADC_ERR_CATCH = false;    // Resets an LED reading if it does not fall within realistic margins. Prevents errors in the filters.
bool ADC_ERR_CAUGHT = false;
bool GET_BASELINE = false;

bool DEBUG_ESP32 = false;
bool DEBUG_ADC = false;       // FOR USE IN A SERIAL MONITOR
bool DEBUG_LEDS = false;
bool SEND_DUMMY_VALUE = false;

bool BLE_ON, BLE_SETUP = false;

const int ledRate = 17000;        // LED flash rate (ms). Can go as fast as 10ms for better heartrate visibility.
const int sampleRate = 8000;      // ADC read rate (ms). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms. Current lib limits it to 125sps
const int samplesPerRatio = 2; // Minimum number of samples per LED to accumulate before making a measurement. Adjust this with your LED rate so you sample across the whole flash at minimum.
const int BTRate = 100000;        // Bluetooth notify rate (ms). Min rate should be 10ms, however it will hang up if the buffer is not flushing. 100ms is stable.
const int USBRate = 0;         // No need to delay USB unless on old setups.

const int nSensors = 1; // Number of sensors (for automated testing)
const int nLEDs = 1; // Number of LED pairs (for automated testing)
// LED GPIO pin definitions. Default LOLIN32 Pinout, commented values are TTGO T1 pins.
int RED = 13, IR = 12 , REDn = 15, IRn = 2; //Default LED GPIO. n values are LEDs used for noise cancelling.
int IR0 = 12;    // Default left 3cm LEDs
int RED0 = 13; //33
int IR1 = 15;//15              // Left 1cm LEDs
int RED1 = 18;//2
int IR2 = 26;             // Right 1cm LEDs
int RED2 = 27;
int IR3 = 19;             // Right 3cm LEDs
int RED3 = 12;

float scaling = 1;

const int LED = 5;  // Lolin32 V1.0.0 LED on Pin 5
const int PWR = 14;//21; // GPIO power to ADC and OPT101 - NOT RECOMMENDED

//SET NON-DEFAULT SDA AND SCL PINS
#define SDA0_PIN 25//23 //5
#define SCL0_PIN 26//19 //18

//For dual i2c or i2c switching
//#define SDA1_PIN ?
//#define SCL1_PIN ?

// HEG VARIABLES
bool coreProgramEnabled = false;
bool adcEnabled = false;
bool reset = false;
bool deviceConnected = false;

String output = "";

int16_t adc0 = 0; // Resulting 15 bit integer.
int16_t lastRead = 0;
int lastLED = 0; // 0 = NO LED, 1 = RED, 2 = IR

//Setup ADS1115
Adafruit_ADS1115 ads(0x48);
long adcChannel = 0; //Channel on the ADC to read. Default 2.

float Voltage = 0.0;
float range = 32767; // 16 bit ADC (15 bits of range)
float gain = 0.256;  // +/- V
float bits2mv = gain / range;

//Signal flags
bool red_led = false; // Bools to alternate LEDS
bool ir_led = false;
bool no_led = true;            // Both LEDS begin off so default is true
bool badSignal = false;        // Bool for too high of an ADC reading
bool signalDetermined = false; // Bool for whether the ADC reading is within desired range

//Counters
int ticks0 = 0, redTicks = 0, irTicks = 0, ratioTicks = 0, noiseTicks = 0, adcTicks = 0, noLEDTicks = 0;

//Scoring variables
long redValue = 0, irValue = 0, rawValue = 0;
float redGet = 0, irGet = 0, redAvg = 0, irAvg = 0, rawAvg = 0, lastRed = 0, lastIR = 0, lastRaw = 0, ratio = 0, baseline = 0;

float lastRatio = 0, v1 = 0, v2 = 0, accel = 0;
float ratioAvg = 0, adcAvg = 0, velAvg = 0, accelAvg = 0;

//TX Variables
//char adcString[10], ratioString[10], posString[10], txString[40]; // Faster in BLE mode
//char scoreString[10]


//Timing variables
unsigned long sampleMicros, currentMicros, LEDMicros, BLEMicros, USBMicros = 0;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Device connected to BLE");
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Device disconnected from BLE");
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      Serial.println("HEG RECEIVE: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }
       
      //commandESP32(rxValue.c_str());

      if (rxValue.find("t") != -1)
      { 
        coreProgramEnabled = true;
        Serial.println("Turning ON!");
      }
      else if (rxValue.find("f") != -1)
      {
        Serial.println("Turning OFF!");
      }
    }
  }
};

void setupBLE()
{
    // Create the BLE Device
    BLEDevice::init("HEG"); // Give it a name
    BLEDevice::setMTU(512);

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    BLE2902 *desc = new BLE2902();
    desc->setNotifications(true);
    pCharacteristic->addDescriptor(desc);

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic->setReadProperty(true);
    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    BLE_SETUP = true;
    BLE_ON = true;
    USE_BT = true;
    Serial.println("BLE service started, scan for HEG.");
}

//Start ADC and set gain. Starts timers
void startADS()
{
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
    sampleMicros = currentMicros;
    LEDMicros = currentMicros;
}

void setupHEG() {
  
  Wire.begin();//SDA0_PIN, SCL0_PIN); // Delete SDA0_PIN, SCL0_PIN if using default SDA/SCL on board
  pinMode(IR, OUTPUT);
  pinMode(RED, OUTPUT);

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);

  //LOLIN32 ONLY
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  if(USE_BT == true){
    setupBLE();
    //SerialBT.begin();
  }

  BLEMicros = currentMicros;
  USBMicros = currentMicros;
}

void sensorTest() { // Test currently selected photodiode and LEDS on 16-bit ADC.
  bool failed = false;

  if (adcEnabled == false) {
    startADS();
  }

  coreProgramEnabled = false;
  digitalWrite(RED, LOW);
  digitalWrite(IR, LOW);
  delay(250);

  for (int j = 0; j < nSensors; j++) {
    adcChannel = j;
    for (int i = 0; i < 10; i++) {
      adc0 = ads.readADC_SingleEnded(adcChannel);

      if (USE_USB == true) {
        Serial.flush();
        Serial.print("Testing photodiode ");
        Serial.print(j);
        Serial.print(" | ADC value: ");
        Serial.println(adc0);
      }
      if (USE_BT == true) {
        /*if (SerialBT.hasClient()) {
          SerialBT.flush();
          SerialBT.print("Testing photodiode " + String(j) + " | ADC value: " + String(adc0) + "\r\n");
        }*/
      }
      if (adc0 == -1) {
        if (USE_USB == true) {
          Serial.println("Error: Check SDA/SCL pin settings and solder connections.");
          Serial.print("SDA pin assigned: ");
          Serial.print(SDA0_PIN);
          Serial.print(" | SCL pin assigned: ");
          Serial.println(SCL0_PIN);
        }
        /*if (USE_BT == true) {
          if (SerialBT.hasClient() == true) {
            SerialBT.print("Error, check SDA/SCL pin settings and solder connections. \r\n");
            SerialBT.print("SDA pin assigned: " + String(SDA0_PIN) + " | SCL pin assigned: " + String(SCL0_PIN) + "\r\n");
          }
        }*/
        failed = true;
      }
      else if ((adc0 > 100) && (adc0 < 32760)) {
        if (USE_USB == true) {
          Serial.println("Photodiode readings within a range that indicates it is working.");
        }
        /*if (USE_BT == true) {
          if (SerialBT.hasClient()) {
            SerialBT.print("Photodiode readings within a range that indicates it is working.\r\n");
          }
        }*/
      }
      else if (adc0 > 32760) {
        if (USE_USB == true) {
          Serial.println("Notice: ADC saturated. Isolate from light or check photodiode pin connections.");
        }
        /*if (USE_BT == true) {
          SerialBT.print("Notice: ADC saturated. Isolate from light or check photodiode pin connections.\r\n");
        }*/
        failed = true;
      }
      delay(100);
    }
  }

  if (failed == true) {
    if (USE_USB == true) {
      Serial.println("Photodiode final test result: FAIL. Resetting...");
    }
    /*if (USE_BT == true) {
      if (SerialBT.hasClient() == true) {
        SerialBT.print("Photodiode final test result: FAIL. Resetting... \r\n");
      }
    }*/
    ESP.restart();
  }
}




void LEDTest() { // Test LEDs assuming photodiodes are good
  bool failed = false;

  if (adcEnabled == false) {
    startADS();
  }

  coreProgramEnabled = false;
  digitalWrite(RED, LOW);
  digitalWrite(IR, LOW);
  delay(250);

  if (USE_USB == true) {
    Serial.flush();
    Serial.println("Testing LEDs...");
  }
  /*if (USE_BT == true) {
    if (SerialBT.hasClient() == true) {
      SerialBT.flush();
      SerialBT.print("Testing LEDs... \r\n");
    }
  }*/

  for (int i = 0; i < nLEDs; i++) {
    if (i == 0) {
      adcChannel = 0;
      RED = RED0;
      IR = IR0;
    }
    if (i == 1) {
      adcChannel = 0; RED = RED1; IR = IR1;
      pinMode(RED, OUTPUT);
      pinMode(IR, OUTPUT);
    }
    if (i == 2) {
      adcChannel = 1; RED = RED2; IR = IR2;
      pinMode(RED, OUTPUT);
      pinMode(IR, OUTPUT);
    }
    if (i == 3) {
      adcChannel = 1; RED = RED3; IR = IR3;
      pinMode(RED, OUTPUT);
      pinMode(IR, OUTPUT);
    }

    for (int j = 0; j < 3; j++) {
      rawValue = ads.readADC_SingleEnded(adcChannel);
      delay(200);
      rawValue = ads.readADC_SingleEnded(adcChannel);
      digitalWrite(RED, HIGH);
      delay(200);
      redValue = ads.readADC_SingleEnded(adcChannel);
      delay(200);
      redValue = ads.readADC_SingleEnded(adcChannel);
      digitalWrite(RED, LOW);
      digitalWrite(IR, HIGH);
      delay(200);
      irValue = ads.readADC_SingleEnded(adcChannel);
      delay(200);
      irValue = ads.readADC_SingleEnded(adcChannel);
      digitalWrite(IR, LOW);
      if (redValue / rawValue < 2) {
        if (USE_USB == true) {
          Serial.flush();
          Serial.print("Error: RED LED difference from ambient insufficient at site: ");
        }
        /*if (USE_BT == true) {
          if (SerialBT.hasClient()) {
            SerialBT.flush();
            SerialBT.print("Error: RED LED difference from ambient insufficient at ");
          }
        }*/
        failed = true;
      }
      if (USE_USB == true) {
        Serial.print("Site: ");
        Serial.println(i);
        Serial.print("Ambient reading: ");
        Serial.print(rawValue);
        Serial.print(" | RED reading: ");
        Serial.println(redValue);
      }
      /*if (USE_BT == true) {
        if (SerialBT.hasClient()) {
          SerialBT.print("Site: " + String(i) + "\r\n" + "Ambient reading: " + String(rawValue) + " | RED reading: " + String(redValue) + "\r\n");
        }
      }*/
      if (irValue / rawValue < 1.1) {
        if (USE_USB == true) {
          Serial.flush();
          Serial.print("Error: IR LED difference from ambience insufficient at ");
        }
        /*if (USE_BT == true) {
          if (SerialBT.hasClient()) {
            SerialBT.flush();          }
        }*/
        failed = true;
      }
      if (USE_USB == true) {
        Serial.print("Site ");
        Serial.println(i);
        Serial.print("Ambient reading: ");
        Serial.print(rawValue);
        Serial.print(" | IR reading: ");
        Serial.println(irValue);
      }
      /*if (USE_BT == true) {
        if (SerialBT.hasClient()) {
          SerialBT.print("Site: " + String(i) + "\r\n" + "Ambient reading: " + String(rawValue) + " | IR reading: " + String(irValue) + "\r\n");
        }
      }*/
      delay(500);
    }
  }

  if (failed == true) {
    if (USE_USB == true) {
      Serial.println("LED final test results: FAIL. Resetting...");
    }
    /*if (USE_BT == true) {
      if (SerialBT.hasClient()) {
        SerialBT.print("LED final test results: FAIL. Resetting..  \r\n");
      }
    }*/
    ESP.restart();
  }
}

void check_signal() {
  if ((adc0 >= 32768) || (reset == true))
  { // The gain is high but anything over 6000 is most likely not a valid signal, anything more than 2000 is not likely your body's signal.
    //Serial.println("\nBad Read ");
    badSignal = true;

    //Temp: reset baseline on bad read
    signalDetermined = false;
    reset = false;
    baseline = 0;

    ticks0 = 0, redTicks = 0, irTicks = 0, noLEDTicks = 0, redValue = 0, irValue = 0, rawValue = 0, 
    redAvg = 0, irAvg = 0, rawAvg = 0, ratioAvg = 0, velAvg = 0, accelAvg = 0;
    redGet = 0, irGet = 0, lastRaw = 0, lastRed = 0, lastIR = 0;
  }
  else if (badSignal == true)
  {
    badSignal = false;
  }
}

void switch_LEDs(int R, int Ir) {
  // Switch LEDs back and forth.
    if ((red_led == false) && ((USE_AMBIENT == false) || (no_led == true))) { // Red on
      if (pIR_MODE == false)
      { // no LEDs in pIR mode, just raw IR from body heat emission.
        digitalWrite(R, HIGH);
        digitalWrite(Ir, LOW);
        red_led = true;
        ir_led = false;
        no_led = false;
        lastLED = 0;
        if (DEBUG_LEDS == true)
        {
          Serial.println("RED_ON");
        }
      }
    }
    else if (red_led == true)
    { // IR on
      if (pIR_MODE == false)
      {
        digitalWrite(R, LOW);
        digitalWrite(Ir, HIGH);
        red_led = false;
        ir_led = true;
        no_led = false;
        lastLED = 1;
        if (DEBUG_LEDS == true)
        {
          Serial.println("IR ON");
        }
      }
    }
    else
    { // No LEDs
      digitalWrite(R, LOW);
      digitalWrite(Ir, LOW);
      red_led = false;
      ir_led = false;
      no_led = true;
      lastLED = 2;
      if (DEBUG_LEDS == true)
      {
        Serial.println("NO_LED");
      }
    }
    LEDMicros = currentMicros;
    //delayMicroseconds(2000); //Let LEDs warm up
    //currentMicros += 2000;
}

void adc_err_catch(){ // Reset a signal reading if it falls outside expected changes compared to previous time.
  ADC_ERR_CAUGHT = false;
  if((lastRed != 0)&&((redGet < (lastRed + lastRed * -0.3)) || (redGet > (lastRed + lastRed * 0.3)))){
    redValue = 0;
    redTicks = 0;
    ADC_ERR_CAUGHT = true;
  }
  if((lastIR != 0)&&((irGet < (lastIR + lastIR * -0.3)) || (irGet > (lastIR + lastIR * 0.3)))){
    irValue = 0;
    irTicks = 0;
    ADC_ERR_CAUGHT = true;
  }
  if(USE_AMBIENT == true){
    if((lastRaw != 0) && ((rawAvg < (lastRaw + lastRaw * -0.3)) || (rawAvg > (lastRaw + lastRaw * 0.3)))){
      rawValue = 0;
      noLEDTicks = 0;
      ADC_ERR_CAUGHT = true;
    }
  }
}

void get_baseline() {
  // GET BASELINE
  if (ticks0 > 100) { // Wait n samples of good signal before getting baseline
    // IR IN 12, RED IN 13
    if ((redTicks < 30) && (irTicks < 30))
    { // && (noLEDTicks < 250)) { // Accumulate samples for baseline
      if (red_led == true)
      { // RED
        redValue += adc0;
        redTicks++;
      }
      else if (ir_led == true)
      { // IR
        irValue += adc0;
        irTicks++;
      }
      else
      {
        rawValue += adc0;
        noLEDTicks++;
      }
      //Serial.println("\nGetting Baseline. . .");
    }
    else
    {
      redGet = (redValue / redTicks); // Divide value by number of samples accumulated 
      irGet = (irValue / irTicks); // Can filter with log10() applied to each value before dividing.
    
      if(USE_AMBIENT == true){
        rawAvg = rawValue / noLEDTicks;
        redGet = redGet - rawAvg;
        irGet = irGet - rawAvg;
      }

      /*if(ADC_ERR_CATCH == true){
        adc_err_catch();
        if(ADC_ERR_CAUGHT == true ){
          return;
        }
        lastRed = redGet;
        lastIR = irGet;
        lastRaw = rawAvg;
      }*/

      baseline = ((redGet) / (irGet)) * scaling; // Get ratio, multiply by 100 for scaling.
      //Serial.println(ratio);
      ratioAvg += baseline;
      redAvg = redGet;
      irAvg = irGet;
      ratioTicks++;

      redValue, irValue, rawValue, redTicks, irTicks, noLEDTicks = 0;
      ratioTicks++;

      signalDetermined = true;
      //Uncomment this for baseline printing
      //Serial.println("\tBaseline R: ");
      //Serial.print(baseline,4);
    }
  }
}

void get_ratio() {
  if ((red_led == true) && (redTicks < samplesPerRatio))
  { // RED
    redValue += adc0;
    redTicks++;
  }
  else if ((ir_led == true) && (irTicks < samplesPerRatio))
  { // IR
    irValue += adc0;
    irTicks++;
  }
  else
  {
    if(noLEDTicks < samplesPerRatio){
      rawValue += adc0;
      noLEDTicks++;
    }
  }
  if ((redTicks >= samplesPerRatio) && (irTicks >= samplesPerRatio) && ((USE_AMBIENT == false) || (noLEDTicks >= samplesPerRatio))) { 
    redGet = (redValue / redTicks); // Divide value by number of samples accumulated // Scalar multiplier to make changes more apparent
    irGet = (irValue / irTicks); // Can filter with log10() applied to each value before dividing.
    
    if(USE_AMBIENT == true){
      rawAvg = rawValue / noLEDTicks;
      redGet = redGet - rawAvg;
      irGet = irGet - rawAvg;
    }
    /*if(ADC_ERR_CATCH == true){
      adc_err_catch();
      if(ADC_ERR_CAUGHT == true){
          return;
      }
      lastRed = redGet;
      lastIR = irGet;
      lastRaw = rawAvg;
    }*/
    lastRatio = ratio;
    ratio = ((redGet) / (irGet)) * scaling; // Get ratio, multiply by 100 for scaling.
    //Serial.println(ratio);
    ratioAvg += ratio;
    redAvg += redGet;
    irAvg += irGet;
    ratioTicks++;

    float t = (currentMicros - sampleMicros) * 0.001;
    if(t > 0){
      v1 = v2;
      v2 = (ratio - lastRatio) / t; // Velocity in adc units/ms
      velAvg += v2;
  
      accel = (v2 - v1) / t; // Acceleration in adc units/ms^2
      accelAvg += accel;
    }
    //score += ratio-baseline; // Simple scoring method. Better is to compare current and last SMA
    //scoreAvg += score;

    ticks0 = 0; //Reset Counters
    redTicks = 0;
    irTicks = 0;
    noLEDTicks = 0;

    redValue = 0; //Reset values to get next average
    irValue = 0;
    rawValue = 0;
  }
}

void readADC(){
  lastRead = adc0;
  if(USE_DIFF == false){
    adc0 = ads.readADC_SingleEnded(adcChannel); // -1 indicates something wrong with the ADC (usually pin settings or solder)
  }
  else{
    if(USE_2_3 == false){
      adc0 = ads.readADC_Differential_0_1();
    }
    else {
      adc0 = ads.readADC_Differential_2_3();
    }
  }
}

// Core loop for HEG sampling.
void core_program(bool doNoiseReduction)
{
    ticks0++;
    if (adcEnabled == false)
    {
      startADS();
    }
    if (SEND_DUMMY_VALUE != true)
    {
      if (currentMicros - sampleMicros >= sampleRate)
      {
        // read the analog in value
        readADC();
        //Voltage = (adc0 * bits2mv);

        // print the results to the Serial Monitor:
        if (DEBUG_ADC == true)
        {
          Serial.print("Last ADC Value: ");
          Serial.print(lastRead);
          Serial.print(", New Value: ");
          Serial.println(adc0);
          //Serial.println("\tVoltage: ");
          //Serial.println(Voltage,7);
        }
        else
        {
          //check_signal();
          if((GET_BASELINE == true)){
            if (signalDetermined == false)
            { // GET BASELINE
              get_baseline();
            }
            else
            { // GET RATIO
              get_ratio();
            }
          }
          else{ // GET RATIO
            get_ratio();
          }
        }
        sampleMicros = currentMicros;
      }
      if (currentMicros - LEDMicros >= ledRate)
      { 
        switch_LEDs(RED, IR); 
      }
      /*
      if(ADC_ERR_CAUGHT == true){
          return;
      }*/
      adcAvg += adc0;
      adcTicks++;
    }
  //DEBUG_ESP32
  if (DEBUG_ESP32 == true)
  {
    if (USE_USB == true)
    {
      Serial.println("Heap after core_program cycle: ");
      Serial.println(ESP.getFreeHeap());
    }
    /*if (USE_BT == true)
    {
      SerialBT.println("Heap after core_program cycle: ");
      SerialBT.println(ESP.getFreeHeap());
    }*/
  }
} // END core_program()


void updateHEG()
{
  if (currentMicros - USBMicros >= USBRate)
  {
    if (adcTicks > 0)
    {
      adcAvg = adcAvg / adcTicks;
      if (ratioTicks > 0)
      {
        ratioAvg = ratioAvg / ratioTicks;
        //outputValue = ratioAvg;
        //output = "NO DATA";
        output = String(currentMicros) + "|" + String(redAvg) + "|" + String(irAvg) + "|" + String(ratioAvg, 4) + "|" + String(rawAvg) + "|" + String(velAvg, 4) + "|" + String(accelAvg, 4) + "\r\n";
        if (USE_USB == true)
        {
          //Serial.flush();
          Serial.print(output);
        }
        if (deviceConnected == true)
        { // could just use output.c_str(); // Need to clean this up
          pCharacteristic->setValue(String(currentMicros).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(redAvg).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(irAvg).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(ratioAvg, 4).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(rawAvg, 4).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(velAvg, 4).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String(accelAvg, 4).c_str());
          pCharacteristic->notify();
          pCharacteristic->setValue(String("|").c_str());
          pCharacteristic->notify();
          delay(10); // bluetooth stack will go into congestion, if too many packets are sent
        }
        /*if (USE_BT == true) //BTSerial
        {
          if (SerialBT.hasClient() == true)
          {
            SerialBT.flush();
            SerialBT.print(String(redAvg) + "|" + String(irAvg) + "|" + String(ratioAvg, 4) + "|" + String(smallFilter.Compute(), 4) + "|" + String(largeFilter.Compute(), 4) + "|" + String(adcAvg, 0) + "|" + String(posAvg, 4) + "\r\n");
            delay(10); // 10ms delay min required for BT output only due to bottleneck in library.
          }
        }*/
      
      }
      adcAvg = 0, redAvg = 0, irAvg = 0, ratioAvg = 0, velAvg = 0, accelAvg = 0, ratioTicks = 0, adcTicks = 0;
      //rawAvg = 0;
    }
    USBMicros = currentMicros;
  }
}

void HEG_core_loop()//void * param)
{
  //while(true){
    if(coreProgramEnabled == true){
      core_program(false);
    }
    updateHEG();
  //}
 //vTaskDelete(NULL);
}
