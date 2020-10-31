To add this chrome extension: 

* Find your Extensions page (chrome://extensions/)
* Enable Developer Mode (top right corner slider usually)
* Select "Load Unpacked" and select this folder (ChromeExtension). 
* To find the extensions after they're loaded, type in chrome://apps in your address bar. 
* You can then right click the icon and create desktop shortcuts. 

This extension enables Serial USB or Bluetooth Serial connectivity to your devices on top of the WiFi features, as well 
as more interactive features (e.g. threeJS) when offline. Pretty cool!

To use the USB connection, click "Get" in the top right of the page if you don't see the "Silicon Labs" 
device option (the HEG), then click "Set" to connect to it and allow you to control it with the 
interface. You can right click the page and click "Inspect" to see the data stream and check errors if 
you are getting any.

You may need the CP210x drivers to recognize the USB connection on your PC:
https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

To use the WiFi connection, connect to your device's WiFi signal (or know its IP on your network if it
is connected to your main), then open up the Data menu and click 'Connect' or type in the custom IP 
first then click.

To use Bluetooth, you must have your device in Bluetooth Serial mode ('B' command or toggled via the 
WiFi Server's Connection Settings page) then pair with it. Then two new Bluetooth COM ports should be
available in the top right port list, the first one should work.

Saved Sessions show up in your default internet Downloads folder.
