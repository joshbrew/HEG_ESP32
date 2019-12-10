// Joshua Brewster - HEG Arduino Functions
// Contributors - Diego Schmaedech (StateChanger Team)
// Requires Arduino ADS1x15 library and Arduino ESP32 library, as well as a compatible ESP32 board

/*
   TODO
   - Fix noise reduction, it never worked
   - HRV basic calculation, adjust LED flash rate accordingly (50ms is viable at bare min) Alternatively, get an ECG chip or use the MAX30102?
   - Accurate SpO2 reading?
   - Check power draw  
   */

#include "ADS1115.h"
//#include <Wire.h>
//#include <Adafruit_ADS1015.h>
#include <esp_timer.h>
//#include <esp_bt.h>
//#include <ArduinoJson.h>

#include "BLE_API.h"
//#include <BluetoothSerial.h>
//#include <SavLayFilter.h>

double filteredRatio;

//SavLayFilter smallFilter(&filteredRatio, 0, 5);  //Cubic smoothing with windowsize of 5
//SavLayFilter largeFilter(&filteredRatio, 0, 25); //Cubic smoothing with windowsize of 25

//Slope variables

const int SLOPE_SIZE = 20;
int slopeCounter = 0;
float slopeArray[SLOPE_SIZE];
void setupStateChanger()
{
  for (int x = 0; x < SLOPE_SIZE; x++)
  {
    slopeArray[x] = 0;
  }
}

bool USE_USB = true;          // WRITE 'u' TO TOGGLE, CHANGE HERE TO SET DEFAULT ON POWERING THE DEVICE
bool USE_BT = false;          // WRITE 'b' TO TOGGLE.
bool pIR_MODE = false;        // SET TO TRUE OR WRITE 'p' TO DO PASSIVE INFRARED ONLY (NO RED LIGHT FOR BLOOD-OXYGEN DETECTION). RATIO IS USELESS HERE, USE ADC CHANGES AS MEASUREMENT.
bool NOISE_REDUCTION = false; // WRITE 'n' TO TOGGLE USING 4 LEDS FOR NOISE CANCELLING *EXPERIMENTAL*
bool USE_DIFF = false;        // Use differential read mode, can reduce noise.
bool USE_2_3 = false;         // Use channels 2 and 3 for differential read.
bool USE_AMBIENT = true;      // Subtract the value of an intermediate no-led reading (good in case of voltage bleeding).
bool ADC_ERR_CATCH = false;   // Resets an LED reading if it does not fall within realistic margins. Prevents errors in the filters.
bool ADC_ERR_CAUGHT = false;

bool DEBUG_ESP32 = false;
bool DEBUG_ADC = false;        // FOR USE IN A SERIAL MONITOR
bool DEBUG_LEDS = false;
bool SEND_DUMMY_VALUE = false;

bool BLE_ON, BLE_SETUP = false;

const int ledRate = 2106;      // LED flash rate (us). Can go as fast as 10ms for better heartrate visibility.
const int sampleRate = 2106;   // ADC read rate (us). ADS1115 has a max of 860sps or 1/860 * 1000 ms or 1.16ms. 
const int samplesPerRatio = 3; // Minimum number of samples per LED to accumulate before making a measurement. Adjust this with your LED rate so you sample across the whole flash at minimum.
const int BTRate = 100000;     // Bluetooth notify rate (us). Min rate should be 10ms, however it will hang up if the buffer is not flushing. 100ms is stable.
const int USBRate = 0;         // No need to delay USB unless on old setups.

const int nSensors = 1; // Number of sensors (for automated testing)
const int nLEDs = 1; // Number of LED pairs (for automated testing)
// LED GPIO pin definitions. Default LOLIN32 Pinout, commented values are TTGO T1 pins.
int RED = 12, IR = 13 , REDn = 15, IRn = 2; //Default LED GPIO. n values are LEDs used for noise cancelling.
int IR0 = 13;    // Default left 3cm LEDs
int RED0 = 12; //33
int IR1 = 15;//15              // Left 1cm LEDs
int RED1 = 18;//2
int IR2 = 26;             // Right 1cm LEDs
int RED2 = 27;
int IR3 = 19;             // Right 3cm LEDs
int RED3 = 12;

float scaling = 1;

const int LED = 5;  // Lolin32 V1.0.0 LED on Pin 5
const int PWR = 14;//21; // Powers ADC and OPT101

//SET NON-DEFAULT SDA AND SCL PINS
#define SDA0_PIN 23 //5
#define SCL0_PIN 19 //18

//For dual i2c or i2c switching
//#define SDA1_PIN ?
//#define SCL1_PIN ?

// HEG VARIABLES
bool coreProgramEnabled = false;
bool adcEnabled = false;
bool reset = false;
bool deviceConnected= false;

String output = "";

int16_t adc0 = 0; // Resulting 15 bit integer.
int16_t lastRead = 0;
int lastLED = 0; // 0 = NO LED, 1 = RED, 2 = IR

//Setup ADS1115
ADS1115 ads(ADS1115_DEFAULT_ADDRESS);
long adcChannel = 0; //Channel on the ADC to read. Default 0.

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
int ticks0, redTicks, irTicks, ratioTicks, noiseTicks, adcTicks, noLEDTicks = 0;

//Scoring variables
long redValue, irValue, rawValue = 0;
float redGet, irGet, redAvg, irAvg, rawAvg, lastRed, lastIR, lastRaw, rednAvg, irnAvg, rawnAvg, ratio, baseline = 0;

float p1, p2, v1, v2, accel = 0;
float ratioAvg, noiseAvg, adcAvg, posAvg, adcnAvg, denoised = 0; //velAvg, accelAvg;

//TX Variables
//char adcString[10], ratioString[10], posString[10], txString[40]; // Faster in BLE mode
//char scoreString[10]


//Timing variables
unsigned long sampleMicros;
unsigned long currentMicros;
unsigned long LEDMicros;
unsigned long BLEMicros;
unsigned long USBMicros;

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

void setupBLE(){

    // Create the BLE Device
    BLEDevice::init("My_HEG"); // Give it a name
    BLEDevice::setMTU(512);

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

    BLE2902 *desc = new BLE2902();
    desc->setNotifications(true);
    pCharacteristic->addDescriptor(desc);

    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic->setReadProperty(true);
    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    BLE_SETUP = true;
    BLE_ON = true;
    USE_BT = true;
    Serial.println("BLE service started, scan for My_HEG.");
}


void readADC(){
  //delayMicroseconds(sampleRate); //Delay so you know a sample happened during a given cycle(on continuous mode)
  lastRead = adc0;
  if(USE_DIFF == false){
    if(adcChannel == 0){
      adc0 = ads.getConversionP0GND();
    }
    if(adcChannel == 1){
      adc0 = ads.getConversionP1GND();
    }
    if(adcChannel == 2){
      adc0 = ads.getConversionP2GND();
    }
    if(adcChannel == 3){
      adc0 = ads.getConversionP3GND();
    }
    //adc0 = ads.readADC_SingleEnded(adcChannel); // -1 indicates something wrong with the ADC (usually pin settings or solder)
  }
  else{
    if(USE_2_3 == false){
      adc0 = ads.getConversionP0N1();
    }
    else {
      adc0 = ads.getConversionP2N3();
    }
  }
  sampleMicros = esp_timer_get_time();
}

void setMux() {
  if(USE_DIFF == false){
    if(adcChannel == 0){
      ads.setMultiplexer(ADS1115_MUX_P0_NG);
    }
    if(adcChannel == 1){
      ads.setMultiplexer(ADS1115_MUX_P1_NG);
    }
    if(adcChannel == 2){
      ads.setMultiplexer(ADS1115_MUX_P2_NG);
    }
    if(adcChannel == 3){
      ads.setMultiplexer(ADS1115_MUX_P3_NG);
    }
  }
  else{
    if(USE_2_3 == false){
      ads.setMultiplexer(ADS1115_MUX_P0_N1);
    }
    else{
      ads.setMultiplexer(ADS1115_MUX_P2_N3);
    }
  }
    //Voltage = (adc0 * bits2mv);
  

}

//Start ADC and set gain. Starts timers
void startADS()
{
    // Begin ADS
    Serial.println("Testing device connections...");
    Serial.println(ads.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");

    Serial.println("Initializing I2C devices..."); 
    ads.initialize(); // initialize ADS1115 16 bit A/D chip
    
      
    // To get output from this method, you'll need to turn on the 
    //#define ADS1115_SERIAL_DEBUG // in the ADS1115.h file
    ads.showConfigRegister();
    
    // We're going to do continuous sampling
    ads.setMode(ADS1115_MODE_SINGLESHOT);
    setMux();
    ads.setRate(ADS1115_RATE_475); //Sample Rate (sps)
    ads.setGain(ADS1115_PGA_0P256); //Step voltage (V)

    adcEnabled = true;
    sampleMicros = esp_timer_get_time();
}

void setupHEG() {

  setupStateChanger();

  pinMode(IR, OUTPUT);
  pinMode(RED, OUTPUT);

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);

  //LOLIN32 ONLY
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

    
  Wire.begin();//SDA0_PIN, SCL0_PIN); // Delete SDA0_PIN, SCL0_PIN if using default SDA/SCL on board


  if(USE_BT == true){
    setupBLE();
    //SerialBT.begin();
  }

  BLEMicros = currentMicros;
  USBMicros = currentMicros;
}


void check_signal() {
  if ((adc0 >= 32000) || (reset == true))
  { // The gain is high but anything over 6000 is most likely not a valid signal, anything more than 2000 is not likely your body's signal.
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
    redGet = 0;
    irGet = 0;
    lastRaw = 0;
    lastRed = 0;
    lastIR = 0;
  }
  else if (badSignal == true)
  {
    output = "Bad Signal";
    badSignal = false;
  }
}

void switch_LEDs(int R, int Ir) {
  // Switch LEDs back and forth.
  if (currentMicros - LEDMicros >= ledRate)
  {
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
          Serial.print("RED_ON\n");
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
          Serial.print("IR ON\n");
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
        Serial.print("NO_LED\n");
      }
    }
    LEDMicros = currentMicros;
    //delayMicroseconds(1000); //Let LEDs warm up
  }
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

      if(ADC_ERR_CATCH == true){
        adc_err_catch();
        if(ADC_ERR_CAUGHT == true ){
          return;
        }
        lastRed = redGet;
        lastIR = irGet;
        lastRaw = rawAvg;
      }

      baseline = ((redGet) / (irGet)) * scaling; // Get ratio, multiply by 100 for scaling.
      //Serial.println(ratio);
      ratioAvg += baseline;
      redAvg = redGet;
      irAvg = irGet;
      ratioTicks++;

      redValue = 0; // Reset values
      irValue = 0;
      rawValue = 0;
      //ticks0 = 0; // Reset counters
      redTicks = 0;
      irTicks = 0;
      noLEDTicks = 0;
      ratioTicks++;

      signalDetermined = true;
      //Uncomment this for baseline printing
      //Serial.println("\tBaseline R: ");
      //Serial.print(baseline,4);
    }
  }
}

void get_ratio(bool isNoise, bool getPos) {
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

    if(ADC_ERR_CATCH == true){
      adc_err_catch();
      if(ADC_ERR_CAUGHT == true){
          return;
      }
      lastRed = redGet;
      lastIR = irGet;
      lastRaw = rawAvg;
    }
    
    ratio = ((redGet) / (irGet)) * scaling; // Get ratio, multiply by 100 for scaling.
    //Serial.println(ratio);
    if (isNoise == false) {
      ratioAvg += ratio;
      redAvg = redGet;
      irAvg = irGet;
      ratioTicks++;
    }
    else {
      noiseAvg += ratio;
      rednAvg = redGet;
      irnAvg = irGet;
      noiseTicks++;
    }

    if (getPos == true) {
      p1 = p2;
      p2 = ratio - baseline; // Position
      posAvg += p2 - p1;
    }

    //v1 = v2;
    //v2 = (p2 - p1) * ticks0 * 0.001; // Velocity in ms
    //velAvg += v2;

    //accel = (v2 - v1) * ticks0 * 0.001; // Acceleration in ms^2
    //accelAvg += accel;

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

// Core loop for HEG sampling.
void core_program(bool doNoiseReduction)
{
    ticks0++;
    if (adcEnabled == false)
    {
      startADS();
      readADC();
      //Start timers
      LEDMicros = currentMicros;
    }
    if (SEND_DUMMY_VALUE != true)
    {
        // Switch LEDs back and forth.
        // Switch LEDs back and forth.
        if(doNoiseReduction == true){
          switch_LEDs(REDn, IRn);
        }
        else {
          switch_LEDs(RED, IR);
        }
        //ads.triggerConversion();
        delayMicroseconds(200);
        // read the analog in value
        readADC();
        delayMicroseconds(2000); // Temp: Signal not stable without fanagling with the delays
        //Voltage = (adc0 * bits2mv);
        // print the results to the Serial Monitor:
        if (DEBUG_ADC == true)
        {
          Serial.print("ADC: " + String(adc0) + "\n");
          //Serial.print(adc0);
          //Serial.println("\tVoltage: ");
          //Serial.println(Voltage,7);
        }
        else
        {
          check_signal();
          if (signalDetermined == false)
          { // GET BASELINE
            get_baseline();
          }
          else
          { // GET RATIO
            if(doNoiseReduction == true){
              get_ratio(true,false);
            }
            else{
              get_ratio(false, true);
            }
          }
        }
      }
      if(ADC_ERR_CAUGHT == true){
          return;
      }
      adcAvg += adc0;
      adcTicks++;
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


//denoising algo. Not final implementation
void denoise(){
  denoised = ratioAvg - noiseAvg;
}

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
        posAvg = posAvg / ratioTicks;
        if((NOISE_REDUCTION == true) && (noiseTicks > 1)){
          noiseAvg = noiseAvg / noiseTicks;
          denoise();
        }
 /*
StateChanger Header Start//==================================================================================================
 */        
        filteredRatio = ratioAvg; //set filter

        slopeArray[slopeCounter] = ratioAvg; //circular buffer
        slopeCounter++;
        float ratioSlope = 0;
        float xAvg = 0;
        float yAvg = 0;
        for (int x = 0; x < SLOPE_SIZE; x++)
        {
          xAvg += x;
          yAvg += slopeArray[x];
        }
        xAvg = xAvg / SLOPE_SIZE;
        yAvg = yAvg / SLOPE_SIZE;
        float v1 = 0;
        float v2 = 0;
        float vAI = 0;
        for (int x = 0; x < SLOPE_SIZE; x++)
        {
          v1 += (x - xAvg) * (slopeArray[x] - yAvg);
          v2 += (float)pow(x - xAvg, 2);
          if (x < (SLOPE_SIZE - 1))
          {
            vAI += (slopeArray[x + 1] - slopeArray[x]) >= 0 ? 1 : 0;
          }
        }
        ratioSlope = abs(v2) > 0 ? (v1 / v2) : 0;
        vAI = (vAI / SLOPE_SIZE) * 100; //ai points
        if (slopeCounter == 20)
        {
          slopeCounter = 0;
        }
        /*
StateChanger Header Start//==================================================================================================
 */
        //outputValue = ratioAvg;
        //output = "NO DATA";
        if(noiseTicks > 0){ 
            output = String(currentMicros) + "|" + String(redAvg) + "|" + String(irAvg) + "|" + String(ratioAvg, 4) + "|" + String(adcAvg, 0) + "|" + String(posAvg, 4) + "|" + String(ratioSlope, 4) + "|" + String(vAI, 4) + "|" + String(denoised, 4) + "\r\n";
            noiseAvg = 0;
            noiseTicks = 0;
        }
        else
        {
          output = String(currentMicros) + "|" + String(redAvg) + "|" + String(irAvg) + "|" + String(ratioAvg, 4) + "|" + String(adcAvg, 0) + "|" + String(posAvg, 4) + "|" + String(ratioSlope, 4) + "|" + String(vAI, 4) + "\r\n";
        }
        
        if (USE_USB == true)
        {
          //Serial.flush();
          Serial.print(output);
        }
        if (deviceConnected)
        {
          pCharacteristic->setValue(output.c_str());
          pCharacteristic->notify();
          delay(10); // bluetooth stack will go into congestion, if too many packets are sent
        }
        /*if (USE_BT == true)
        {
          if (SerialBT.hasClient() == true)
          {
            SerialBT.flush();
            SerialBT.print(String(redAvg) + "|" + String(irAvg) + "|" + String(ratioAvg, 4) + "|" + String(smallFilter.Compute(), 4) + "|" + String(largeFilter.Compute(), 4) + "|" + String(adcAvg, 0) + "|" + String(posAvg, 4) + "\r\n");
            delay(10); // 10ms delay min required for BT output only due to bottleneck in library.
          }
        }*/
      }
      //rawAvg = 0;
      ratioAvg = 0;
      posAvg = 0;
      adcAvg = 0;
      //adc0 = 0;
      ratioTicks = 0;
      adcTicks = 0;
    }
    USBMicros = currentMicros;
  }
}

void HEG_core_loop()//void * param)
{
  //while(true){
    if(coreProgramEnabled == true){
      if(ratioTicks == 0) {
        core_program(false);
      }
      else { // Need to make sure the mux changes so the continuous conversions swap channels
        if(NOISE_REDUCTION == true) {    
          core_program(true);
        }
      }
    }
    if(NOISE_REDUCTION == true) {
      if(noiseTicks > 0){
        updateHEG(); 
      }
    }
    else {
      updateHEG();
    }
  //}
 //vTaskDelete(NULL);
}
