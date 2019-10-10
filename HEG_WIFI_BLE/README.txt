HEG WiFi WIP implementation. 

*****TO FLASH THIS SKETCH*****

You need the github version of the Arduino ESP32 libraries, follow steps accordingly for your OS.

Arduino's default partition settings for the ESP32 need to be changed.

You can change the partition scheme to "Minimal SPIFFS" in the Arduino Tools menu.

Alternatively, to have increased SPIFFs:

In the Arduino>hardware>espressif>esp32 folder replace boards.txt with the one in this library

In esp32>tools>partitions replace default.csv with the one in this library

These change the default partitioning to allow sketches up to 2MB in size on the ESP32 to make room for dual WiFi and BLE.

Extra SPIFFS notes:
You need the SPIFFs tools: https://github.com/me-no-dev/arduino-esp32fs-plugin
You also need to copy the esptool and mkspiffs EXE files in Arduino/hardware/espressif/esp32/tools/esptool and tools/mkspiffs into the
upper /tools directory so the SPIFFs tool can find it.


*****
In Documents/Arduino/hardware/espressif/esp32/cores/esp32, open main.cpp and change 
xTaskCreateUniversal(loopTask, "loopTask", 8196, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
to
xTaskCreateUniversal(loopTask, "loopTask", 16384, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);

The heap memory needs to be increased as the change in wifi is too much on for the default arduino config.
*****

After flashing this sketch onto the ESP32, you will find the new wifi
access point at the SSID: ESPWebServer with password: 12345678.

After logging into the access point, access the interface at 192.168.4.1

--DEVICE INSTRUCTIONS--

HEG serial commands:
't' - Turns sensor on, you'll see a data stream if the sensor is isolated, as in on your forehead and not saturated by ambient light.
'f' - Sensor off
'b' - Toggle BLE mode, this resets the device to start in BLE mode, so you have to enter 'b' again through the serial or BLE connection to switch back to WiFi.
'R' - hard resets the ESP32
's' - soft resets the sensor data
'u' - toggles USB data stream. 
'p' - really basic pIR setting. Just turns the LEDs off as the photodiode picks up radiant heat from your body.
'0','1','2','3' - Changes ADC channel the device reads, in the case of multiple light sensors.
'D' - toggles ADC debugging (serial only)

With extra sensors:
'l' - toggles sensor 0 and LED set 0.
'c' - toggles sensor 0 and LED set 2.
'r' - toggles sensor 1 and LED set 3.
'n' - broken and basic noise reduction implementation, where two LED sites are compared. This is not a good implementation yet, may cause device to hang.

Output data stream:

Current Milliseconds | Red LED Sample Average | IR LED Sample Average |
Red/IR Ratio Average | small Sav Lay Filter | large Sav Lay Filter |
adc Avg | position Average | ratio Slope | Attention Index

With noise average activated, the denoised ratio (if using an extra pair of LEDs) is inserted between Red/IR Ratio Avg and the small Sav Lay Filter.

The following demo pages are available:
/       - Index page, basic site navigation.
/sc     - State Changer demo page.
/listen - HEG Output event listener test. Better than websockets for this use case.
/stream - HEG Output websocket stream test. May crash the device.
/update - Upload compiled binaries and flash the ESP32 over the web.
/connect - connect HEG to router so it may be accessed via a router instead. 
-------

Wifi code notes:

Use the /connect page to save Wifi credentials to EEPROM. The ESP32 will reset and attempt connection. 
If connection fails, the access point opens (StateChanger in your WiFi scanner).

The function setupStation(hostname, pword) crashes. It is disabled on the /connect page at the moment until I figure out a fix.
The fix is to use vTaskCreate or vTaskCreateUniversal to call setupStation, so it splits the thread in memory, then wait for that task to finish before advancing in handleDoConnect();

BLE and WiFi do not work concurrently at the moment, logging into WiFi with BLE enabled will cause the ESP32 to crash.
Use the serial 'b' command for now to test switching modes. Will add an html toggle as well on next update.

mDNS should allow the ESP32 to be accessed via http://esp32.local. It requires Bonjour service enabled. I have not had success yet, but it works on Apple systems apparently.


-------

Serial Port notes:
Chrome app serial monitor (alternative to Arduino): https://chrome.google.com/webstore/detail/serial-monitor/ohncdkkhephpakbbecnkclhjkmbjnmlo/related?hl=en

-------

HEG Code notes:

The noise_reduce() function in this version is still broken. That needs to be triggered.

On the /listen or /stream page once the event listener/websocket is connected you can control and test the HEG through the Serial monitor. Right now there is a manual 75ms delay on the data stream, I'm going to create buffers as the new sensors will be WAY faster than TCP can handle.

Changelog:
10/10/19
-----
CSV Save/Load functions added.
Graphing improved, canvas demo fairly playable now. 
Waiting on new pcbs to get definitive data after fixing noise issues in designs.

TODO:
graph upgrades
reformat functions and variables into respective classes to reduce bloat.
sliders for exploring data
create SPIFFS js packages? That way we can break up essential functions on the webAPI and reduce bloat or add third party packages easily.
hegstudio brightness exercise recreation is easiest to do with default <video> tag and a transparency overlay.
Fix spaghetti

10/6/19
-----
Fixed crash when changing wifi modes
Add static IP settings. 
Working on WebGL based graphing and video controls. /hegvid wip


10/1/19
------
Canvas demo
Really basic navigation on index page
Fix websocket page to use js to call the hostname in case of esp32.local being used.
More POST commands, also added text input-based command method as an option, only send single char.

9/9/19
------
EventSource works great!

More TODO:
Make connect page notify the user of the new IP address if esp32.local is not fixed.
Convert websockets to drive commandESP32

9/5/19
------
TODO:
Data visualizer. HEG Life game in canvas?
Stress testing - try EventSource instead of Websockets. Websockets can overflow and crash the ESP32 easily if there is lag clientside.
Login interface?
HTML interface to use commandESP32

9/5/2019
------
Fixed OTA, update boards.txt and default.csv in their respective folders (see above install instructions)
Changed setup loop so wifi is always chosen if EEPROM.read(0) is not 1 for BLE.


9/3/19
------
Included BLE support. toggle BLE/WIFI with 'b', BLE default off due to conflict with WiFi
Websockets and HEG now on separate threads so sensor speed is not impacted - WS speed set at 13.33 updates/sec as it's stable. Max = 16/sec
/update broken. Gotta fix it.
Lots of housekeeping to keep workflow easy, tested the freeRTOS threading system a ton.
commandESP32 now in WiFi_API.h.
