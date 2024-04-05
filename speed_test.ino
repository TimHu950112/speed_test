#include <Arduino.h>
#include "soc/rtc_wdt.h"
#include <LiquidCrystal_I2C.h> 


LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

volatile int count = 0;
unsigned long lastMillis = 0;
float frequency = 0;
float rpm = 0;
float max_rpm = 0;

void IRAM_ATTR hallSensorISR() {
  count++;
}

void setup() {
  Serial.begin(115200);
  
  lcd.init(); // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM");
  lcd.setCursor(0,1);
  lcd.print("STARTED");
  delay(3000);
  lcd.clear();
  
  pinMode(12, INPUT); // Hall sensor input pin with internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(12), hallSensorISR, FALLING); // Interrupt on falling edge

  // Create tasks once
  xTaskCreatePinnedToCore(detectSpeed, "DetectSpeedTask", 10000, NULL, 1, NULL, 0); // Core 0
  xTaskCreatePinnedToCore(calculateSpeed, "CalculateSpeedTask", 10000, NULL, 1, NULL, 1); // Core 1
  disableCore0WDT(); 
}

void loop() {
  // Empty loop
}

void detectSpeed(void *parameter) {
  for (;;) {
    lastMillis = millis();
    while (millis() - lastMillis < 1000) {
      // Wait for 1 second to count the number of pulses
    }
  }
}

void calculateSpeed(void *parameter) {
  for (;;) {
    float elapsedTime = (millis() - lastMillis) / 1; // Elapsed time in seconds
    frequency = count / elapsedTime; // Frequency in Hz
    rpm = frequency * 60; // RPM calculation
    if (rpm > max_rpm){
      max_rpm=rpm;
    }
    Serial.print("max_rpm:"+String(max_rpm));
    Serial.print("Frequency (Hz): ");
    lcd.setCursor(0,0);
    lcd.print("Frequency:");
    lcd.setCursor(11,0);
    lcd.print(String((frequency/3)*1000));
    Serial.print(frequency/3);
    lcd.setCursor(0,1);
    lcd.print("RPM: ");
    lcd.setCursor(11,1);
    lcd.print(String(rpm/3));
    Serial.print(" - RPM: ");
    Serial.println(rpm/3);
    count = 0; // Reset count for next interval
    vTaskDelay(pdMS_TO_TICKS(1000)); // Non-blocking delay
    lcd.clear();
  }
}
