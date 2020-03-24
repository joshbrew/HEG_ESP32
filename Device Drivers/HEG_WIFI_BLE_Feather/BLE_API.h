#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

BLECharacteristic *pCharacteristic;

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

bool USE_BT = false;          // WRITE 'B' TO TOGGLE.
bool USE_BLE = false;         // WRITE 'b' TO TOGGLE.
bool BLE_ON, BLE_SETUP = false;
bool deviceConnected = false;

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

void setupBTSerial() {
  SerialBT.begin("HEGduino BT");
  Serial.println("HEG booted in Bluetooth Serial Mode! Pair it with your device and access the stream via standard serial monitor.");
  USE_BT = true;
  delay(100);
}
