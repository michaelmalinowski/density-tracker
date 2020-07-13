#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void printDevices(){
    lcd.clear();
    lcd.print("Active Devices:");
    lcd.setCursor(0,1);
    while (Serial.available() > 0) {
        char data = Serial.read();
        lcd.print(data);
    }
}

void setup(){
    Serial.begin(115200);
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("Scanner Screen");
}

void loop(){
    delay(1000);
    Serial.print("1");
    
    printDevices();
    
}
