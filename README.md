# HEG ESP32 - Open Source. Home of HEGduino!
Hemoencephalography meets highly affordable IoT! Now with WiFi and bluetooth! Incoming schematics, apps, and how-to's. [New Delobotomizer repo here](https://github.com/moothyknight/HEG_ESP32_Delobotomizer) for drivers for the new hardware.

Join our little community at: [HEG Alpha](https://hegalpha.com)

Important HEGduino stuff you can find here:
- [HEG Training Quick Start Guide](https://github.com/moothyknight/HEG_ESP32/blob/master/Guides/HEG%20Training%20Quick%20Start%20Guide.pdf)
- [Progressive webapp (WIP) for Desktop or Android. Use with Chrome for now](https://hegalomania.netlify.app)
- [HEGduino Assembly](https://github.com/moothyknight/HEG_ESP32/blob/master/Guides/HEGduino%20Assembly%20Guide.pdf)
- [(For self-assembly only) Setting up Arduino to flash the ESP32 with our biofeedback software](https://github.com/moothyknight/HEG_ESP32/blob/master/Device_README.txt)
- [Updating the firmware via the web interface](https://github.com/moothyknight/HEG_ESP32/blob/master/Guides/How%20To%20Update.pdf)
- [Lolin32 Firmware and Updates](https://github.com/moothyknight/HEG_ESP32/tree/master/Device%20Drivers/HEG_WIFI_BLE_Lolin32)
- [Huzzah32 Feather Firmware and Updates](https://github.com/moothyknight/HEG_ESP32/tree/master/Device%20Drivers/HEG_WIFI_BLE_Feather)

## What is it?
Hemoencephalography is a single sensor FNIRS device meant to give a basic indication of cerebral blood flow changes in the brain.
Users then combine this indicator - the ratio of red to infrared light returned to the photodiode from the LEDs on your scalp or forehead - with simple visualization tools to help them increase or decrease metabolic/blood flow activity in the brain. 
This works like an odd form of physical therapy and is implicated as a powerful tool when combined in any brain health related therapies.

![Explanation](https://raw.githubusercontent.com/moothyknight/HEG_ESP32/master/Pictures/hegbiofeedback.png)

## Software

### Arduino Setup
See the [Device_Readme.txt](https://github.com/moothyknight/HEG_ESP32/blob/master/Device_README.txt)  for flashing instructions as well as available USB/WiFi/BLE commands and the changelog, including which dependencies are required for the Arduino IDE. We packaged the more minor dependencies in the Device_Drivers folder so you don't have to chase all of them down, just get Arduino and the ESP32 addon (github version) working.

Use the HEG_WIFI_BLE sketch in the arduino IDE on your respective ESP32. Pin definitions in HEG.h for the SDA/SCL and LEDs need to be adjusted according to your setup. 

These builds are tested on the Lolin32 and Huzzah32 Feather respectively but should work on any board after modifying the pinouts in HEG.h

### All Open Source Games, Tools, & APIs. See [the new PWA!](https://hegalomania.netlify.app)

<img src="https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/platforms.PNG" alt="platforms" width="252" height="354">

On the firmware you will find a Async Web Server with a cross-platform supported javascript app, enabling plug-and-play and global networking features. We are creating an open source combined therapy and research toolset, using the perks of an online-enabled device and all of the diverse tools available for web front and backend. This is a living project so stay tuned!

You can demo it now at a new mobile or desktop-installable webapp at [hegalomania.netlify.app](https://hegalomania.netlify.app). 

The USB support on the PWA is chrome-only until it is out of development for default web support. Install on desktop by opening the setting dropdown in Chrome (the 3 vertical dots in the top right of the browser) and click "Install HEG Alpha." It should be kept up to date automatically and work offline. Make sure you have the Earth textures on the Sunrise mode cached to use them offline, simply load them once to do this.  

It's evolving fast. We will eventually be comparable to professional BCI software, free of charge. In the More folder, please find the DataCharting.html applet for analyzing and comparing your data with interactive charts.

![Screenshot](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/HEGwebAPI.png?raw=true)

## Data Charter (To be replaced) CSV analyzer for the web app (also found on our [Data page on our website](https://hegalpha.com))
![HEGCharter](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/datacharter.PNG?raw=true)

## More Links
Our website: [HEG Alpha](https://hegalpha.com)

Find us on [CrowdSupply](https://crowdsupply.com/alaskit/hegduino)

and [OpenBCI](https://shop.openbci.com/collections/frontpage/products/hegduino-kit?variant=32052268531784)

Special thanks to the [Biocomp/Biofeedback Institute of LA](https://www.biocompresearch.org/), [Brain Trainer](https://brain-trainer.com/), and [AlasKit](https://alaskit.net)

See also (very deprecated):
[Arduino Nano V3 HEG](https://github.com/moothyknight/HEG_Arduino)

