# HEG_ESP32 - Open Source
Now on CrowdSupply!!! This includes an article elaborating on the science: https://crowdsupply.com/alaskit/hegduino

Hemoencephalography meets highly affordable IoT! Now with bluetooth! Incoming schematics, app, and how-to's. [Join the Slack!](https://join.slack.com/t/hegopensource/shared_invite/enQtMzg4ODAzODQxMzY1LWUyOGU4N2ZiM2EwM2Y1YzJmMmU0YWFkY2YyMWI1NGJmODA3ZjczOGM0NzI3MjAwOTJkYjY1MTU1MmRmYTJkMjM)

We have patched the original open source HEGstudio by Jonathan Toomim to work with our HEG! 

v0.1.2 with EXE, Tested for Windows 7, 8, and 10: https://files.fm/f/78ufkvzy

Source available in this repo, but requires the resources and media files available from the original sourceforge: https://sourceforge.net/projects/hegstudio/

See also:
[Arduino Nano V3 HEG](https://github.com/moothyknight/HEG_Arduino)

## Materials
- Arduino Huzzah32 Feather or Lolin32 V1.0.0. Other ESP32 arduino boards are compatible but usually have different pinouts.
- ADS 1115 w/ PGA, 16-bit ADC
- OPT101 Photodiode
- 1 HAN1102W Infrared LED
- 1 BR1102W Red LED
- 6 pin SOT23 socket (for mounting LEDs)
- MicroUSB cable. 

You can save quite a bit of money if you use Ebay or Aliexpress for the ESP32, ADS1115, and OPT101. Mouser or Digikey should have the LEDs, note the HAN1102W is the latest version of the AN1102W and is easier to find. You can get a prototype together for less than 15 dollars buying singular components (before shipping) if you know what you are doing.

## What is it?
Hemoencephalography is a device that allows you to measure and influence control over the bloodflow in your brain. It's just like any other pulse ox, but allows for a type of physical brain exercise. HEGs typically cost hundreds or thousands on the market, so this is a much better solution for people wanting to get their feet wet with biofeedback and do a cool DIY project to understand the extremely straightforward science better. [HEG biofeedback](https://en.wikipedia.org/wiki/Hemoencephalography) was originally developed as a safe and non-invasive method to treat ADD in the late 90s, later expanding to treating disorders like PTSD and Depression due to common stress symptoms like [Hypofrontality](https://en.wikipedia.org/wiki/Hypofrontality) being measurable with this tool. 

It is implicated for much more, but there's not a whole lot of data (which this 20 dollar version could solve). HEG biofeedback is informally called "brain pushups" for how it enables one to literally work out their brain to enable better bloodflow and oxygenation, therefore cognitive functioning and self-control. It only takes 5-10 minutes in the first session to gain control of bloodflow in the targeted area. I don't recommend doing more than 10 minutes at a time with this thing. Be wary of fatigue or headaches the first few times when you start gaining control of your bloodflow. View the [RecommendedReading.txt](https://github.com/moothyknight/HEG_Arduino/blob/master/RecommendedReading.txt) for some resources to educate yourself more deeply on this subject.

## Explanation
![Explanation](https://raw.githubusercontent.com/moothyknight/HEG_Arduino/master/Pics/HEGExplained.png)

Placeholder...
## Schematics
![Schematic](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/HEG_ESP32Arduino_BP.png?raw=true)

## Assembly
We used cheap protoboard to mount our DIY prototypes. For the sensor we used a protoboard snapped in half and taped together for a pseudo flexible circuit.

1. Cut your wire leads to be the desired length between the sensor, which goes on your forehead, and the receiver. Sensor-only or ADC to ESP32 leads should be the shortest distance, which will vary for different setups so use your judgment.

2. Wire it up based on the above schematics diagram. The anode marks on the LEDs are green. Use a 6 pin SOT23 socket to mount the SMT LEDs, they should only be a few cents. Use electrical tape to cover any pins or wires exposed so they do not contact your forehead.

3. Install ESP32 firmware on device via Arduino IDE. Requires [ADS1x15 library](https://github.com/adafruit/Adafruit_ADS1X15) and [arduino ESP32 library](https://github.com/espressif/arduino-esp32) 

4. Secure sensor to forehead, make sure ALL light is blocked as the photodiode is very sensitive.

5. The data output via serial USB or bluetooth (toggleable in the script or via serial) is structured as: "ADC_READ, RATIO_READ, POSITION_FROM_BASELINE" The Ratio section will transmit "WAIT" when it is still gathering samples, including when getting the initial baseline. 

We'll soon be adding free Windows and Android apps, including compatibility with the original HEGstudio by Jonathan Toomim!

### Testing Notes
The photodiode is very sensitive. Any moisture or stray light will throw off the readings. Block off any light in-between and around the LEDs and the photodiode to ensure no leaks.

The maximum ADC reading is 32768 (15 bits) and each step is 0.125mV at the maximum 16X gain on the ADS1115. This is plenty sensitive for blood-oxygen detection. The range for real data is in the 200-3000 range. The script will not set the baseline unless it is below a specified light threshold.

Be sure to cover the photodiode pins and the LED contacts with electrical tape so your forehead doesn't complete the circuit and throw off readings.

If there is moisture expect ADC or score readings to decline to zero and even negative values.

If your IR LED is not working the ratio will be positive as the red LED has a lower intensity. Ratio = Red / Infrared. You can reduce the flash rate in the arduino script to test conclusively via the ADC.



## App Features
React Native with SPP or custom BLE support.
Still bare-bones at this point. Working on multi-threading.
Packages for Android and iOS incoming!

## Example Data
Graph 1 is about 1 min of ratio data.
Graph 2 is about 4 min of scoring data (a simple ratio - baseline, will be changed to the cumulative change in the SMA)
Graph 3 is about 3 seconds of ADC data at 20hz LED flash frequency.
![data](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/Screenshot_2019-01-23-21-18-36.jpg?raw=true)

## HEGstudio with this HEG
![](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/20190211_201736.jpg?raw=true)
