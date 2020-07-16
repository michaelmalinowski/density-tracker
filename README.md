# density-tracker

## Introduction 

Understanding foot traffic or daily location population can be important for many people. Creating a device that is cheap and can send data collected to a server is vital.

Using an esp8266, this code will allow you to analyze packets in your vicinity, retrieve mac addresses and determine active devices. 

This data can either be displayed onto an lcd screen or be sent over uart to another esp8266. The receiving esp8266 can process the data and send it to a 
another device over Wifi.

## How It Works
Every device that has it's wifi turned on is constantly looking for "better networks". An indication of a better network is signal strength. Every 30-60 seconds our 
smart phones are looking for "better networks" by sending probe request using the wifi 802.11 frames. Certain devices can capture these frames and extract data from them. 

### Devices Connected to Network
Probe requests contain unique identifiers call MAC addresses. Each phone has a different address that it uses to separate itself from other devices. While connected to a wifi
network, this address doesnt change and each packet analyzed with a different MAC address can be associated as a unique active device. If a ping from this MAC address is
not seen for a specific amount of time, it can be deemed as inactive. 

### Devices Not Associated to a Network
Devices that are not connected to a network are sending probe requests more often. However, newer OS have implemented a feature that randomizes MAC addresses of none connected 
devices to protect their privacy. In this situation a new approach needs to be adopted to determine activate devices. A burst analyzes solution was used in which time frame are
allocated for probe request. Consecutive time frames are averaged out to estimate the amount of devices. This approach will not distinguish between conencted and not conencted.

## Getting Started
![alt text](https://github.com/michaelmalinowski/density-tracker/blob/master/density_tracker_schematic.PNG "Wiring Schematic")

**The resistors are both 330 omhs**

The receiver ESP8266 is not necessary. 

Upload the code to an ESP8266 wire according to the diagram. On start up, `Blacklist Mode?` should appear on the lcd with a count down. The black list mode is used to exclude
device from being considered an active device. *Devices conencted to a network can only be blacklisted*. An application for this mode is if you want to exclude office devices 
from being considered. To enter blacklist mode press the button and a counter will appear with how many devices have been blacklisted. Press the button again to enter scanning 
mode. 

At this point, data will either be sent to the lcd or a different device depending on what option is selected.




## Understanding the Variables

| Options                           | Description          | Value  | Defaults |
| -------------                     |:-------------:| -----:| --------:|
| `WIFI_ON`                         | Selecting between scan mode. 0 for scanning all devices. 1 for scanning devices only connected to wifi |0 or 1| `WIFI_ON=0`  |
| `BLACKLIST_MODE_COUNTDOWN`        | The amount of seconds before the options to enter blacklist mode ends |> 0| `BLACKLIST_MODE_COUNTDOWN=10` |
| `DEVICE_TIMEOUT`                  | The amount of minutess before a device is deemed inactive (Only if `WIFI_ON=1`) |> 0|`DEVICE_TIMEOUT=5`|
| `DEVICE_TIMEOUT_SCAN`             | The amount of minutes before a scan is trigger to check for expired devices (Only if `WIFI_ON=1`)|> 0|`DEVICE_TIMEOUT_SCAN=1`|
| `BURST_DURATION`                  | The amount of seconds before a burst scan ends |> 0|`BURST_DURATION=30`|
| `BURST_AVERAGE`                   | The amount of bursts to average across |> 0|`BURST_DURATION=2`|
| `SEND_DATA`                       | Selecting between data mode. 0 for sending data to lcd. 1 for sending data over Serial |0 or 1|`SEND_DATA=0`|

## Future Development
A website is currently being developed to share gathered data amongst the community. Our main goal is to raise awareness of population density at certain locations. 
