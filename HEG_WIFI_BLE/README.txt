HEG WiFi WIP implementation. 

*****TO FLASH THIS SKETCH*****

You need the github version of the Arduino ESP32 libraries, follow steps accordingly for your OS.

You need to change the partition scheme to "Minimal SPIFFS" in the Arduino Tools menu.

*****
IMPORTANT STEP
In Documents/Arduino/hardware/espressif/esp32/cores/esp32, open main.cpp and change 
xTaskCreateUniversal(loopTask, "loopTask", 8196, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
to
xTaskCreateUniversal(loopTask, "loopTask", 16384, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
The heap memory needs to be increased as the change in wifi can be too much on for the default arduino config. 
*****

After flashing this sketch onto the ESP32, you will find the new wifi
access point at the SSID: My_HEG with password: 12345678.

After logging into the access point, access the interface at 192.168.4.1

********************************

**
Optional - to have increased SPIFFs:

Arduino's default partition settings for the ESP32 need to be changed. A shell script has been provided to speed this up.

In the Arduino>hardware>espressif>esp32 folder replace boards.txt with the one in this library

In esp32>tools>partitions replace default.csv with the one in this library

These change the default partitioning to allow sketches up to 2MB in size on the ESP32 to make room for dual WiFi and BLE.

Extra SPIFFS notes:
You need the SPIFFs tools: https://github.com/me-no-dev/arduino-esp32fs-plugin
You also need to copy the esptool and mkspiffs EXE files in Arduino/hardware/espressif/esp32/tools/esptool and .../tools/mkspiffs into the
upper .../tools directory so the SPIFFs tool can find them.
**

--DEVICE INSTRUCTIONS--

HEG serial commands:
't' - Turns sensor on, you'll see a data stream if the sensor is isolated, as in contacting your skin and not exposed to ambient light.
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
/connect - connect HEG to router so it may be accessed via a router instead. 
/update - Upload compiled binaries and flash the ESP32 over the web.

-------

Wifi code notes:

Use the /connect page to save Wifi credentials to EEPROM. The ESP32 will reset and attempt connection. 
If connection fails, the access point opens (StateChanger in your WiFi scanner).

BLE and WiFi do not work concurrently at the moment, logging into WiFi with BLE enabled may cause the ESP32 to crash.
Use the serial 'b' command for now to test switching modes or use the /connect page options.

mDNS should allow the ESP32 to be accessed via http://esp32.local. It requires Bonjour service enabled. I have not had success yet, but it works on Apple systems apparently.


-------

Serial Port notes:
Chrome app serial monitor (alternative to Arduino): https://chrome.google.com/webstore/detail/serial-monitor/ohncdkkhephpakbbecnkclhjkmbjnmlo/related?hl=en
Arduino IDE has a debugger that comes with the ESP32 libs if there are some weird crashes happening.
-------

HEG Code notes:

The noise_reduce() function in this version is still broken. That needs to be triggered.

On the /listen or /stream page once the event listener/websocket is connected you can control and test the HEG through the Serial monitor. Right now there is a manual 75ms delay on the data stream, I'm going to create buffers as the new sensors will be WAY faster than TCP can handle.

*-*-*-*-*-*-*
Changelog:
*-*-*-*-*-*-*
10/21/19
------
-proof of concept hill climb game
-more cleanup (not all the way there yet)

More to do:
Audio mode that just plays a tone that gets louder or quieter? 
Device search on local wifi when in STA mode.
More controls for modes.

10/18/19
------
-integrated audio mode
-added volume feedback for video option
-code feng shui improvements

FYI: not all buttons hooked up yet

Known bugs:
-not every browser works on canvas demo (they should)
-offset button bugged when offsetting more than the number of graph vertices (look at xoffsetSlider.onclick setting)
10/17/19
------
smooth mode switching on demo page. Audio and hill climbing (LIFE game re-creation) games incoming.
change graph to show cumulative SMA 1s - 2s slope changes. This math is a simple noise filter that is still reactive to the current information, which can overcome noise issues. 
split code up into multiple files to fit a cleaner traditional webapp format.
web api improving and generalizing rapidly
can set primary/secondary dns if needed, smart config type stuff incoming.

couple bugs:
X offset slider still, it's 3 lines of code causing a problem.
the replay function works fine but goes over the index when resetting the data. This bit will be re-done as the code is spaghetti.

10/14/19
------
-x offset bar. Has a bug when offsetting when graph scale is lower than the array being sliced. page has to be reloaded if red line vanishes.
-Even classier, almost finished streamlining code.
-integrated hegvid code with main page, need to create options to swap once finished generalizing the data handling. this will be smoooooth


10/13/19
------
webGL graphing API improvements. More to come.
WIP Object-oriented web API on canvas demo page. Final suite will be completely class-based.
Sensitivity slider.
/hegvid -> /video (this page will be integrated with the main demo)


More TODO:
Audio visualizer (HTML5 has sweet built-in tools)
CSS Overhaul w/ animations. Utilize webkit/moz/mobile libs.
Fix mobile support for web demo (mostly requires permissions changes)

10/11/19
-----
/hegvid page improvements/experiments. Can load local video files (web URLs next, some site embedding like Vimeo MIGHT work)


10/10/19
-----
CSV Save/Load functions added.
Graphing improved, canvas demo fairly playable now. 
Waiting on new pcbs to get definitive data after fixing noise issues in designs.
GraphJS wrapper class for graphing.

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
