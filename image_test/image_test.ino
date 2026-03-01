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
 if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
   Serial.println(F("SSD1306 allocation failed"));
   for(;;); // Don't proceed, loop forever
 }
 display.clearDisplay(); 
}

void loop() {
  for (i=0; i<31; i++){
   display.clearDisplay();
   int perc = map(i, 0, 30, 0, 8);
   display.drawBitmap(0, 0, epd_bitmap_allArray[perc], 128, 64, WHITE);
   display.setCursor(0,0);            /* Set x,y coordinates */
   display.println(i); /* Text to be displayed */
   display.display();
   delay(100);
  }
}
