#include <ezButton.h>  // the library to use for SW pin
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define CLK 34
#define DATA 35
#define SW_PIN 32

#define DIRECTION_CW 0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction


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

bool setupRotary(){
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  
  return true;
}


/// DISPLAY
void oled_volumeDisplay(int vol){
    display.clearDisplay();

    display.setCursor(0,0);
    display.println("VOLUME"); //Display current setting
    
    int perc = map(vol, 0, 30, 0, 8); //map volume between 0-8 to set aperture display

    
    display.drawBitmap(0, 0, epd_bitmap_allArray[perc], 128, 64, WHITE);
    display.setCursor(60,36);
    display.println(vol); //Display current volume
    display.display();
}

/// ROTARY
static uint8_t prevNextCode = 0;
static uint16_t store=0;
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[prevNextCode] ) {
      store <<= 4;
      store |= prevNextCode;
      if ((store&0xff)==0x2b) return -1;
      if ((store&0xff)==0x17) return 1;
   }
   return 0;
} 

void setup() {
  Serial.begin (115200);
  setupRotary();
  setupDisplay();
}

void loop() {
  static int8_t c,val;

   if( val=read_rotary() ) {
      c +=val;
      if(c > 30){
        c=0;
        } else if (c < 0){
          c=0;
          }
      Serial.println(c);
      oled_volumeDisplay(c);
   }
}
