# HEG_ESP32 - Open Source
Now on CrowdSupply!!! https://crowdsupply.com/alaskit/hegduino

Hemoencephalography meets highly affordable IoT! Now with bluetooth! Incoming schematics, app, and how-to's. [Join the Slack!](https://join.slack.com/t/hegopensource/shared_invite/enQtMzg4ODAzODQxMzY1LWUyOGU4N2ZiM2EwM2Y1YzJmMmU0YWFkY2YyMWI1NGJmODA3ZjczOGM0NzI3MjAwOTJkYjY1MTU1MmRmYTJkMjM)

See also:
[Arduino Nano V3 HEG](https://github.com/moothyknight/HEG_Arduino)

## Materials
- Arduino Huzzah32 Feather. There are at least a dozen alternatives, this one has a battery. I'm also doing a Lolin32 build
- ADS 1115 w/ PGA, 16-bit ADC
- OPT101 Photodiode
- 1 AN1102W Infrared LED
- 1 BR1102W Red LED
- MicroUSB cable. 

## What is it?
HEGs typically cost hundreds or thousands on the market, so this is a much better solution for people wanting to get their feet wet with biofeedback and do a cool DIY project to understand the extremely straightforward science better. [HEG biofeedback](https://en.wikipedia.org/wiki/Hemoencephalography) was originally developed as a safe and non-invasive method to treat ADD in the late 90s, later expanding to treating disorders like PTSD and Depression due to common stress symptoms like [Hypofrontality](https://en.wikipedia.org/wiki/Hypofrontality) being treatable with this tool. It is implicated for much more, but there's not a whole lot of data (which this 20 dollar version could solve). It is informally called "brain pushups" for how it works out your brain to enable better bloodflow and oxygenation, therefore cognitive functioning and self-control. It only takes 5-10 minutes in the first session to gain control of bloodflow in the targeted area. I don't recommend doing more than 10 minutes at a time with this thing. Be wary of fatigue or headaches the first few times when you start gaining control of your bloodflow. View the [RecommendedReading.txt](https://github.com/moothyknight/HEG_Arduino/blob/master/RecommendedReading.txt) for some resources to educate yourself more deeply on this subject.

## Explanation
![Explanation](https://raw.githubusercontent.com/moothyknight/HEG_Arduino/master/Pics/HEGExplained.png)

Placeholder...
## Schematics
![Schematic](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/HEG_ESP32Arduino_BP.png?raw=true)

## App Features
React Native with SPP or custom BLE support
Still bare-bones at this point.
Packaged project incoming!

## Example Data
Graph 1 is about 1 min of ratio data.
Graph 2 is about 4 min of scoring data (ratio - baseline)
Graph 3 is about 3 seconds of ADC data at 20hz LED flash frequency.
![data](https://github.com/moothyknight/HEG_ESP32/blob/master/Pictures/Screenshot_2019-01-23-21-18-36.jpg?raw=true)
