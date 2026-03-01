#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  setupDisplay();
}

bool setupDisplay(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay(); 
  return true;
}

void oled_volumeDisplay(int vol){
    display.clearDisplay();

    display.setCursor(0,0);
    display.println("VOLUME"); //Display current setting
    
    int perc = map(i, 0, 30, 0, 9); //map volume between 0-9 to set aperture display

    
    display.drawBitmap(64, 40, epd_bitmap_allArray[perc], 128, 64, WHITE);
    display.setCursor(64,40);
    display.println(i); //Display current volume
    display.display();
}

void loop() {
  for (i=0; i<31; i++){
    oled_volumeDisplay(i);
    delay(100);
  }
}
