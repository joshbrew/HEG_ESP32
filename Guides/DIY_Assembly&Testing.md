## Materials
- Arduino Huzzah32 Feather ($18) or Lolin32 V1.0.0 ($4). Other ESP32 arduino boards are compatible but usually have different pinouts. We also really like the TTGO T1 for its SD card support at ~$5 per board (i.e. remote data collection).
- ADS 1115 w/ PGA, 16-bit ADC
- OPT101 Photodiode
- 1 HAN1102W or DN1102W Infrared LED
- 1 BR1102W Red LED
- 6 pin SOT23 socket (for mounting LEDs)
- MicroUSB cable. 

You can save quite a bit of money if you use Ebay or Aliexpress for the ESP32 dev board, ADS1115, and OPT101. Mouser or Digikey should have the LEDs, note the HAN1102W is the latest version of the AN1102W and is easier to find. DN1102W is preferred at 850nm for IR by Biocomp as it is has fairly identical reflectivity between Hb or HbO2, providing a better baseline. You can get a prototype together for less than 15 dollars buying singular components (before shipping) if you know what you are doing.

## Schematics
![Schematic](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/HEG_ESP32Arduino_BP.PNG?raw=true)

## Assembly
We used cheap perfboard to mount our DIY prototypes. For the sensor we used a protoboard snapped in half and taped together for a pseudo flexible circuit.

1. Cut your wire leads to be the desired length between the sensor, which goes on your forehead, and the receiver. Sensor-only or ADC to ESP32 leads should be the shortest distance, which will vary for different setups so use your judgment.

2. Wire it up based on the above schematics diagram. The anode marks on the LEDs are green. Use a 6 pin SOT23 socket to mount the SMT LEDs, they should only be a few cents. Use electrical tape to cover any pins or wires exposed so they do not contact your forehead. Ensure each component is grounded separately to minimize noise and crosstalk (made that mistake).

3. Install ESP32 firmware on device via Arduino IDE. Requires [ADS1x15 library](https://github.com/adafruit/Adafruit_ADS1X15) and [arduino ESP32 library](https://github.com/espressif/arduino-esp32) 

4. Secure sensor to forehead, make sure ALL light is blocked as the photodiode is very sensitive.

5. The data output via WiFi, serial USB, or bluetooth (toggleable in the script or via serial) is structured as: Current Milliseconds | Red LED Sample Average | IR LED Sample Average |
Red/IR Ratio Average | small Sav Lay Filter | large Sav Lay Filter |
adc Avg | position Average | ratio Slope | Attention Index

Data will not transmit if the photodiode is saturated, use command 'T' to test diode or activate debug options.

We'll soon be adding free Windows and Android apps. We have compatibility with the original HEGstudio by Jonathan Toomim! It's hacky but the feedback outputs more or less are identical to the Peanut.

### Testing Notes
The photodiode is very sensitive. Any moisture or stray light will throw off the readings. Block off any light in-between and around the LEDs and the photodiode to ensure no leaks.

The maximum ADC reading is 32768 (15 bits) and each step is 0.125mV at the maximum 16X gain on the ADS1115. This is plenty sensitive for blood-oxygen detection. The range for real data is in the 200-3000 range. The script will not set the baseline unless it is below a specified light threshold.

An ADC read of -1 means the ADC is not selected on the right pins (or those pins aren't soldered), or unpowered, or even fried. If you are however receiving random values with no responsiveness your A0 pin connection to the photodiode needs to be checked. A constant value of 32768 with no responsiveness to light means there is a short or the 1MΩ pin might not be connected to the signal output of the photodiode.

Be sure to cover the photodiode pins and the LED contacts with electrical tape so your forehead doesn't complete the circuit and throw off readings.

If there is moisture expect ADC or score readings to decline to zero and even negative values.

If your IR LED is not working the ratio will be positive as the red LED has a lower intensity. Ratio = Red / Infrared. You can reduce the flash rate in the arduino script to test conclusively via the ADC.
