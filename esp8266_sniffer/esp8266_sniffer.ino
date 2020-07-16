extern "C" {
	#include <user_interface.h>
}

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>

#define ON_WIFI 0 //scanning devices connected to a network or not  
#define SCAN_CHANNEL 6
#define BLACKLIST_MODE_COUNTDOWN 10 //seconds to press button to enter blacklist mode 

//ON_WIFI=1
#define DEVICE_TIMEOUT 5 //scanned device timeout in minutes
#define DEVICE_TIMEOUT_SCAN 1 //timeout checker in minutes 

//ON_WIFI=0
#define BURST_DURATION 30 //duration of a burst in seconds
#define BURST_AVERAGE 2 // how many bursts to average over

#define SEND_DATA 0 //0 for lcd function and 1 for sending data to another board

struct frameControl {
    unsigned protocol: 2;
    unsigned type: 2;
    unsigned subtype: 4;
    unsigned toDS: 1;
    unsigned fromDS: 1;
    unsigned frag: 1;
    unsigned retry: 1;
    unsigned pwrManage: 1;
    unsigned moreData: 1;
    unsigned proFrame: 1;
    unsigned order: 1;
};

struct rxControl {
	signed rssi:8; 
	unsigned rate:4;
	unsigned is_group:1;
	unsigned:1;
	unsigned sig_mode:2;
	unsigned legacy_length:12;
	unsigned damatch0:1;
	unsigned damatch1:1;
	unsigned bssidmatch0:1;
	unsigned bssidmatch1:1;
	unsigned MCS:7; 
	unsigned CWB:1; 
 	unsigned HT_length:16;
 	unsigned Smoothing:1;
 	unsigned Not_Sounding:1;
 	unsigned:1;
 	unsigned Aggregation:1;
 	unsigned STBC:2;
 	unsigned FEC_CODING:1;
 	unsigned SGI:1;
 	unsigned rxend_state:8;
 	unsigned ampdu_cnt:8;
 	unsigned channel:4; 
 	unsigned:12;
};

struct snifferStream {
    struct rxControl rx_control;
    uint8_t buf[112];
    uint16_t count;
    uint16_t length;
};

struct device {
	uint8_t mac[6];
	unsigned long last_seen;
};

struct deviceList {
	device list[200];
	uint8_t len = 0;
};

static deviceList device_list;
static deviceList black_list;

uint8_t bursts[BURST_AVERAGE];
uint8_t burst_number = 0;
uint8_t previous_burst_total = 0;

static unsigned long current_time;
static unsigned long clear_time;
const char MAC_FORMAT[] = "%02X:%02X:%02X:%02X:%02X:%02X\n";
bool mode = false;

LiquidCrystal lcd(2,0,4,5,13,12);


//checks if mac address is within the list and return the mac address if it isnt

//prints rssi and channel from rxControl to serial
void printRxControl(rxControl data){
	Serial.printf("rssi: %d\n", data.rssi);
	Serial.printf("channel: %d\n", data.channel);
}

//Keeps the device in blacklist mode until a button press changes the mode
void blacklistMode(){
	bool buttonPressed = false;
	wifi_init();
	while(mode){
		lcd.setCursor(0,1);
		buttonPressed = (digitalRead(15) == HIGH);
		delay(100);
		if (buttonPressed) {
			mode = false;
			lcd.clear();
			lcd.print("Exited");
			delay(500);
			break;
		}
		lcd.print(black_list.len);
		lcd.print(" ");
	}
}

//prints three mac addresses to serial
void printMacAddresses(char* mac1, char* mac2, char* mac3){
	Serial.printf("Receiver: ");
	Serial.printf(MAC_FORMAT, mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5]);
	Serial.printf("Sender: ");
	Serial.printf(MAC_FORMAT, mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
	Serial.printf("Filter: ");
	Serial.printf(MAC_FORMAT, mac3[0], mac3[1], mac3[2], mac3[3], mac3[4], mac3[5]);
}

//saves a mac address to a deviceList 
void saveMacAddress(uint8_t* mac, deviceList* devices, rxControl data, bool print=false){
	uint8_t i;
	if (!mode) {
		for (i = 0; i < devices->len; ++i){
			if (memcmp(mac, devices->list[i].mac, 6) == 0) {
				devices->list[i].last_seen = current_time;
				return;
			}
		}
	}
	if (blacklistedDevice(mac)) {
		device new_device;
		memcpy(new_device.mac, mac, 6);
		new_device.last_seen = current_time;
		devices->list[devices->len] = new_device;
		devices->len += 1;
		if (print) {
			Serial.printf(MAC_FORMAT, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			printRxControl(data);
		}
	}
	return;
}

//checks if a device has expired with a deviceList
uint8_t expiredDevices(deviceList* devices=&device_list){
	//Serial.printf("Expired device check\n");
	uint8_t i;
	uint8_t shift = 0, expired = 0;
	for(i = 0; i < devices->len; ++i){
		if (shift > 0) {
			devices->list[i - shift] = devices->list[i];
		}
		if (current_time - devices->list[i].last_seen > DEVICE_TIMEOUT * 60000) {
			++expired;
			++shift;
		}
	}
	devices->len -= expired;
	return expired;
}

//checks if a device is in the blacklist
bool blacklistedDevice(uint8_t* mac){
	uint8_t i;
	for (i = 0; i < black_list.len; ++i){
		if (memcmp(mac, black_list.list[i].mac, 6) == 0) {
			black_list.list[i].last_seen = current_time;
			return false;
		}
	}
	return true;
}

//callback function.
//Analyzes packet data
void packetAnalyzer(uint8_t* buf, uint16_t len){
	if (len == 12) {
		return;
	} else if (len == 128) {
		uint8_t mac1[6];
		uint8_t mac2[6];
		uint8_t mac3[6];
		snifferStream *data = (snifferStream*) buf;
		frameControl *frame = (frameControl*) data->buf;
		//Probe request
		if (frame->type == 0 && frame->subtype == 4){
			uint8_t i;
			for (i = 0; i < 6; ++i) {
				mac1[i] = data->buf[i + 10];
			}
			if (mode == 0) {
				
				saveMacAddress(mac1, &device_list, data->rx_control);
			}
		//probe response
		} else {
			uint8_t i;
			for (i = 0; i < 6; ++i) {
				mac1[i] = data->buf[i + 4];
			}
			for (i = 0; i < 6; ++i) {
				mac2[i] = data->buf[i + 10];
			}
			for (i = 0; i < 6; ++i) {
				mac3[i] = data->buf[i + 16];
			}

		} 
		if (mode) {
			saveMacAddress(mac1, &black_list, data->rx_control);
		}
	} 
}

//formats device mac address into a single uint8_t array
void formatDevices(uint8_t* deviceArray){
	uint8_t i, k, indexPoint;
	for (i = 0; i < device_list.len; ++i) {
		indexPoint = i * 6;
		for (k = 0; k < 6; ++k) {
			deviceArray[indexPoint + k] = device_list.list[i].mac[k];
		}
	}
}

//prints num to an lcd
void printDevices(uint8_t num){
    lcd.clear();
    lcd.print("Active Devices:");
    lcd.setCursor(0,1);
    lcd.print(num);
    delay(250);
}

//prints num to serial
void sendDevices(uint8_t num){
	if(Serial.available() > 0){
		char incomingData = Serial.read();
		if (incomingData == '1') {
			Serial.printf("%d", num);
		}
	}
}

//transmits device number data 
void transmitData(uint8_t num, uint8_t type=SEND_DATA){
	if (type) {
		sendDevices(num);
	} else {
		printDevices(num);
	}
}

//wifi initilatization function  
void wifi_init(){
	wifi_set_opmode(STATION_MODE);
	wifi_set_channel(SCAN_CHANNEL);
	wifi_set_promiscuous_rx_cb(packetAnalyzer);
	delay(100);
	wifi_promiscuous_enable(1);
}

void setup(){
	Serial.begin(115200);
	delay(100);
	//LCD INIT
	lcd.begin(16, 2);
    lcd.print("Scanner Screen");
	delay(250);
	lcd.clear();
	//WIFI 
	lcd.print("BlackList Mode?");
	current_time = millis();
	bool buttonPressed = false;
	while(BLACKLIST_MODE_COUNTDOWN > (current_time/1000)){
		current_time = millis();
		lcd.setCursor(0,1);
		lcd.print((long)floor((BLACKLIST_MODE_COUNTDOWN - (current_time/1000))));
		lcd.print(" ");
		buttonPressed = digitalRead(15);
		if (buttonPressed) {
			lcd.clear();
			lcd.print("BlackListed:");
			mode = true;
			break;
		}
		yield();
	}

	blacklistMode();
	lcd.clear();

	lcd.print("Scanner Started");
	clear_time = current_time;

}

void loop(){
	yield();
	current_time = millis();
	if (current_time - clear_time > DEVICE_TIMEOUT_SCAN * 60000 && ON_WIFI) {
		clear_time = current_time;
		expiredDevices();
		transmitData(device_list.len);
	}
	if (ON_WIFI == 0) {
		if (current_time - clear_time > BURST_DURATION * 1000) {
			bursts[burst_number] = device_list.len;
			device_list.len = 0;
			++burst_number;
			clear_time = current_time;
		}
		if (burst_number >= BURST_AVERAGE) {
			burst_number = 0;
			uint8_t i;
			int average = 0;
			for (i = 0; i < BURST_AVERAGE; ++i){
				average += bursts[i];
			}
			previous_burst_total = (uint8_t)average/BURST_AVERAGE;
		}	
		transmitData(previous_burst_total);
	}
}
