# Mesh Security
The mesh needs to be secure such that it can not be accesable to the outside. This can be configured and monitored via a few ways.


## Channel and Range
The painlessMesh uses the WiFi channel 2.4Ghz and can be configured with its own channel, default it is operating on channel 1.
Without taking any external factors or major obstacles a lot of standard WiFi signals and systems that operate on the 2.4GHz frequency can reach up to 46 meters indoors and 92 meters outdoor. 

For range testing results I kindly refer to our <a>indoor test</a>.

When operating in the 2.4 GHz band, it is recommended to use the smaller 20 MHz WiFi channel width. The major reason for this is that there are many overlapping channels in this band. (8 of the 11 channels overlap). As we all know, overlapping WiFi channels are a major source of network disturbance. As a result, if you pick a greater channel width that connects numerous overlapping channels in this band, you will have poorer wireless performance.

If there are overlapping WiFi channels or signals this also be configured with an other channel (previrbaly 1 or 11).

This setting can be done in the <a href="https://gitlab.com/painlessMesh/painlessMesh/-/blob/develop/src/arduino/wifi.hpp"> wifi.hpp file </a> of the painlessMesh library.

## Security Protocols
For the basic connection towards the WiFi it inherits the WiFi class combind with its own init() configuration.

When compared with the basic <a href="https://github.com/arduino-libraries/WiFi/blob/master/examples/ConnectWithWPA/ConnectWithWPA.ino">ConnectWithWPA.ino</a> example and with some research the following things can be concluded about security:

- A WEP network can have four separate keys, each with its own "key index" value. The SSID, key, and key number are required for WEP encrypted networks. **Luckily this is not used in the painless library** because this is also an old and redundant technique.


- WPA/WPA2 (TKIP) is used when the connection is made toward the Home Assistant API (**STA MODE**). WPA3 is currently being developed by esp-idf.

- ESP has native hardware encryption for AES in **AP mode** so this has been imported automattically in order to add an extra security layer to messages passed between nodes in the mesh.
