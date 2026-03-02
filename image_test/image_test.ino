/**#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  setupDisplay();
}

bool setupDisplay(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  return true;
}

void oled_volumeDisplay(int vol){
    display.clearDisplay();

    display.setCursor(0,0);
    display.println("VOLUME"); //Display current setting
    
    int perc = map(vol, 0, 30, 0, 9); //map volume between 0-9 to set aperture display

    
    display.drawBitmap(0, 0, epd_bitmap_allArray[perc], 128, 64, WHITE);
    display.setCursor(60,36);
    display.println(vol); //Display current volume
    display.display();
}

void loop() {
  for (int i=0; i<31; i++){
    oled_volumeDisplay(i);
    delay(100);
  }
}**/
