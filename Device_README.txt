See below the instructions for the device commands and notes on device initialization and connecting.

*****TO FLASH THIS SOFTWARE ON YOUR ESP32*****

HEG ESP32 WiFi/BLE implementation. Do not follow these instructions unless you are building from scratch or intend to modify 
the firmware yourself, otherwise use our .bin files provided and flash your respective ESP32 model via the web interface provided.

You need Arduino IDE: https://www.arduino.cc/en/Main/Software
You may need the CP210x Drivers from Silicon Labs: https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

In Arduino IDE install the espressif ESP32 libraries via the manager. You don't need to modify the source files anymore. 

You need to select the correct board via the Boards menu and change the partition scheme to "Minimal SPIFFS" in the Arduino Tools menu.

Other Required Libraries (found in the Device Drivers folder via Arduino_Dependencies.zip): 
AsyncTCP 
ESPAsyncWebServer 
ADS1X15
ArduinoJson


**
Optional - to have increased SPIFFs:

Arduino's default partition settings for the ESP32 need to be changed. A shell script has been provided to speed this up.

In the Arduino>hardware>espressif>esp32 folder replace boards.txt with the one in this library

In esp32>tools>partitions replace default.csv with the one in this library

These change the default partitioning to allow sketches up to 2MB in size on the ESP32 to make room for dual WiFi and BLE.

Extra SPIFFS notes:
To use SPIFFs files you need the SPIFFs tools: https://github.com/me-no-dev/arduino-esp32fs-plugin
You also need to copy the esptool and mkspiffs EXE files in Arduino/hardware/espressif/esp32/tools/esptool and .../tools/mkspiffs into the
upper .../tools directory so the SPIFFs tool can find them.
**

--DEVICE INSTRUCTIONS--

When the device boots up you will see LEDs blink, first in a rapid series, then 3 times slowly. These indicate which mode you are in then pressing the reset button or pulling the plug on the device between the slow blinks will let you do different things.

First set of rapid blinks indicates communication modes:
Two blinks - Bluetooth Low Energy mode indicator
Three blinks - WiFi mode indicator
Four blinks - Bluetooth classic mode indicator

Then resetting/unpowering the device between the indicator and slow LED blinks:
After rapid blinks - put device to sleep
After first slow blink - change communication mode (see above for codes)
After second blink - Reset WiFi credentials, sometimes it does not connect to routers correctly and then kaputs the WiFi mode - or you haven't found the IP which is visible on the serial monitor or in your router settings.
After third blink - Normal operation

HEG serial/ble/wifi commands:
't' - Turns sensor on, you'll see a data stream if the sensor is isolated, as in contacting your skin and not exposed to ambient light.
'f' - Sensor off.
'F' - toggle filters
'X' - toggle DC filter
'W' - Reset WiFi to default access point mode (wipes saved credentials).
'u' - toggles USB only streaming. Toggles back to WiFi mode (default)
'B' - Toggle Bluetooth Serial mode. Resets device to be in BTSerial mode. Enter 'B' again to swap back. Can connect to this via standard serial monitors.
'b' - Toggle BLE mode, this resets the device to start in BLE mode, so you have to enter 'b' again through the serial or BLE connection to switch back to WiFi.
'R' - hard resets the ESP32.
'S' - places ESP32 in deep sleep mode, reset power (press the reset button on the device) to re-activate
's' - soft resets the sensor data.
'o' - fast output mode for android web BLE (20 byte max packet sizes).
'p' - really basic pIR setting. Just turns the LEDs off as the photodiode picks up radiant heat from your body.
'0','1','2','3' - Changes ADC channel the device reads, in the case of multiple light sensors.
'5' - Read differential between A0 and A1 on ADS1115 to reduce noise (e.g. connect A1 to signal ground).
'6' - Read differential between A2 and A3 on ADS1115.
'D' - toggles ADC debugging (serial only).
'L' - toggles LED ambient light cancellation .

With extra sensors:
'l' - toggles sensor 0 (channel 0) and LED set 0.
'c' - toggles sensor 0 (channel 0) and LED set 2.
'r' - toggles sensor 1 (channel 2) and LED set 3.

Output data stream:

Current Microseconds | Red LED Sample Average | IR LED Sample Average |
Red/IR Ratio Average | Ambient Sample | Velocity (ratio change/ms)| Acceleration (1 = ratio change/ms^2)

The following demo pages are available:
/       - Index page, basic site navigation.
/listen - HEG Output event listener test. Better than websockets for this use case.
/connect - connect HEG to router so it may be accessed via a router instead. 
/update - Upload compiled binaries and flash the ESP32 over the web.

-------

Wifi code notes:

Use the /connect page to save Wifi credentials to EEPROM. The ESP32 will reset and attempt connection. 
If connection fails, the access point opens (My_HEG in your WiFi scanner).

BLE and WiFi do not work concurrently at the moment, logging into WiFi with BLE enabled may cause the ESP32 to crash.
Use the serial 'b' command for now to test switching modes or use the /connect page options.

mDNS should allow the ESP32 to be accessed via http://esp32.local. It requires Bonjour service enabled. I have not had success yet, but it works on Apple systems apparently.


-------
Serial Port notes:
Chrome app serial monitor (alternative to Arduino): https://chrome.google.com/webstore/detail/arduino-chrome-serial-mon/opgcocnebgmkhcafcclmgfldjhlnacjd?hl=en
Arduino IDE has a debugger that comes with the ESP32 libs if there are some weird crashes happening.
-------

HEG Code notes:

On the /listen or /stream page once the event listener/websocket is connected you can control and test the HEG through the Serial monitor. Right now there is a manual 50ms delay on the data stream, I'm going to create buffers as the new sensors will be WAY faster than TCP can handle.
