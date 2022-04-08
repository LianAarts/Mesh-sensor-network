# Mesh Security
The mesh needs to be secure so that it cannot be accessed from the outside. We can prevent and monitor unwanted access by implementing some features.

## Channel and Range
The painlessMesh uses the 2.4Ghz WiFi band. Channel **one** is used by default but this can be changed. 
Systems that operate on the 2.4GHz frequency can have a range that reaches up to 46 meters indoors and 92 meters outdoor. External factors can influence the range.

For range testing results I kindly refer to our <a>indoor test</a>.

When operating in the 2.4 GHz band, it is recommended to use the smaller 20 MHz channel width. The major reason for this is that there are many overlapping channels in this band (8 of the 11 channels overlap). As we all know, overlapping WiFi channels are a major source of network disturbance. As a result, if you pick a greater channel width that connects numerous overlapping channels in this band, you will have poorer wireless performance.

We can use other channels if there are overlapping ones. Channels one and eleven are preferred.

This setting can be changed in the <a href="https://gitlab.com/painlessMesh/painlessMesh/-/blob/develop/src/arduino/wifi.hpp"> wifi.hpp file </a> of the painlessMesh library.

## Unwanted Access

Code that is uploaded to the chip cannot be accessed. There is no way to retrieve source code from the ESP32.

## Security Protocols
The basic wifi connection is inherited from the WiFi class and its own **init()** configuration

When compared to the basic <a href="https://github.com/arduino-libraries/WiFi/blob/master/examples/ConnectWithWPA/ConnectWithWPA.ino">ConnectWithWPA.ino</a> example and with some research the following things can be concluded about security:

- A WEP network can have four separate keys, each with its own "key index" value. The SSID, key, and key number are required for WEP encrypted networks. **Luckily this is not used in the painless library** because this is also an old and redundant technique.


- WPA2 (TKIP) is used when the connection is made toward the Home Assistant API (**STA MODE**). Unfortunately, WPA3 is currently being developed by esp-idf and is not available for us.

- ESP has native hardware encryption for AES in **AP mode** so this has been imported automatically to add an extra security layer to messages passed between nodes in the mesh.

- The ESP32-VROOM-32 chip uses the **IEEE 802.11 b/g/n standard** 