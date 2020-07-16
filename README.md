# density-tracker

## Introduction 

Physical distancing becomes increasingly difficult with larger groups within an area. Creating a cheap device that tracks foot traffic to inform users and can send data
collected to a server is vital.

Using an esp8266, this code will allow you to analyze packets in your vicinity, retrieve mac addresses and determine active devices. 

This data can either be displayed onto an LCD screen or be sent over UART to another esp8266. The receiving esp8266 can process the data and send it to a 
another device over Wifi.

## How It Works
Every device that has it's wifi turned on is constantly searching for "better networks" in the background. An indication of a better network is signal strength. Every 30-60
seconds our smart phones do so by sending probe requests using the wifi 802.11 data frames. Certain devices can capture these frames and extract data from them. 

### Devices Connected to Network
Probe requests contain unique identifiers call MAC addresses. Each phone has a different address that it uses to distinguish itself from other devices. While connected to a wifi
network, this address doesn't change and each packet analyzed with a different MAC address can be associated as a unique active device. If a ping from this MAC address is
not seen for a specific amount of time, it can be deemed as inactive. 

### Devices Not Associated to a Network
Devices that are not connected to a network send probe requests more often. However, newer OS have implemented a feature that randomizes MAC addresses of non-connected 
devices to protect their privacy. In this situation a new approach needs to be adopted to determine active devices. A burst analysis solution was used in which time frames are
allocated for probe requests. Consecutive time frames are averaged out to estimate the amount of devices. This approach will not distinguish between connected and non-connected
devices.

## Getting Started
![alt text](https://github.com/michaelmalinowski/density-tracker/blob/master/density_tracker_schematic.PNG "Wiring Schematic")

**The resistors are both 330 omhs**

The receiver ESP8266 is not necessary. 

Upload the code to an ESP8266 wire according to the diagram. On start up, `Blacklist Mode?` should appear on the LCD with a count down. The black list mode is used to exclude
devices from being considered as active. *Devices connected to a network can only be blacklisted*. An application for this mode is if you want to exclude office devices (ie.
wifi printers, desktops, security cameras) from being considered. To enter blacklist mode press the button and a counter will appear with how many devices have been blacklisted.
Press the button again to enter scanning mode (scanning active devices). 

At this point, data will either be sent to the LCD or a different device depending on what option is selected.




## Understanding the Variables

| Options                           | Description          | Value  | Defaults |
| -------------                     |:-------------:| -----:| --------:|
| `WIFI_ON`                         | Selecting between scan mode. 0 for scanning all devices. 1 for scanning devices only connected to wifi |0 or 1| `WIFI_ON=0`  |
| `BLACKLIST_MODE_COUNTDOWN`        | The amount of seconds before the options to enter blacklist mode ends |> 0| `BLACKLIST_MODE_COUNTDOWN=10` |
| `DEVICE_TIMEOUT`                  | The amount of minutes before a device is deemed inactive (Only if `WIFI_ON=1`) |> 0|`DEVICE_TIMEOUT=5`|
| `DEVICE_TIMEOUT_SCAN`             | The amount of minutes before a scan is trigger to check for expired devices (Only if `WIFI_ON=1`)|> 0|`DEVICE_TIMEOUT_SCAN=1`|
| `BURST_DURATION`                  | The amount of seconds before a burst scan ends |> 0|`BURST_DURATION=30`|
| `BURST_AVERAGE`                   | The amount of bursts to average across |> 0|`BURST_DURATION=2`|
| `SEND_DATA`                       | Selecting between data mode. 0 for sending data to lcd. 1 for sending data over UART |0 or 1|`SEND_DATA=0`|

## Future Development
A website is currently being developed to share gathered data amongst the community. The goal is to inform users on how busy their destinations are (ie. grocery store, park) so
they can plan accordingly, such as deciding on an alternative destination, to encourage physical distancing and avoid risk from crowds.
