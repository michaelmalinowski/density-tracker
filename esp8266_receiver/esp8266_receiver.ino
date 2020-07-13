#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "wifiSecrets.h"

#include <Arduino.h>


void setup() {
 Serial.begin(115200);                 //Serial connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);   //WiFi connection
 
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
 
    delay(500);
    Serial.println("Waiting for connection");
 
  }
}

void requestData(){
    int i;
    char incomingData[3];

    Serial.printf("%d", 1);
    delay(100);

	while (Serial.available() > 0){
		incomingData[i] = Serial.read();
		Serial.printf("%d", incomingData);
        ++i;
        if (i > 2) {
            break;
        }
	}
    if (i > 0) {
        sendData(incomingData);
    }
}

void sendData(char* data){
    //Send the data to anywhere from here
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
        Serial.println("Connected");
        HTTPClient http;    //Declare object of class HTTPClient

        http.begin("http://www.google.ca");      //Specify request destination
        http.addHeader("Content-Type", "text/plain");  //Specify content-type header

        int httpCode = http.GET();   //Send the request
        String payload = http.getString();                  //Get the response payload

        Serial.println(httpCode);   //Print HTTP return code
        //Serial.println(payload);    //Print request response payload

        http.end();  //Close connection

    } else {
        Serial.println("Error in WiFi connection");   
    }
}

void loop() {
    delay(10000);
    requestData();
}