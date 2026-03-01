// For a connection via I2C using Wire include
#include "DFRobot_OLED12864.h" 

// Include custom images
#include "images.h"


// Initialize the OLED display using Wire library
DFRobot_OLED12864  display(0x3c);


#include <ezButton.h>  // the library to use for SW pin

#define CLK 34
#define DATA 35
#define SW_PIN 32

#define DIRECTION_CW 0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

void setup()
{

  display.init();
  display.flipScreenVertically();// flip vertical
  display.clear();
  display.display();

  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println();
  Serial.println();

}

// DISPLAY
void drawPerc(int perc){
  switch (perc) {
        case 0:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_0);
          display.display();
          break;
        case 13:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_13);
          display.display();
          break;
        case 25:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_25);
          display.display();
          break;
        case 36:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_38);
          display.display();
          break;
        case 50:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_50);
          display.display();
          break;
        case 63:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_63);
          display.display();
          break;
        case 75:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_75);
          display.display();
          break;
        case 88:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_88);
          display.display();
          break;
        case 100:
          display.drawXbm(0, 0, Picture_width, Picture_height, perc_100);
          display.display();
          break;
      }
    }

// ROTARY 
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
      //if (store==0xd42b) return 1;
      //if (store==0xe817) return -1;
      if ((store&0xff)==0x2b) return -1;
      if ((store&0xff)==0x17) return 1;
   }
   return 0;
}



void loop()
{
  // read rotary values | cap between 0 and 100
  static int8_t c,val;
  if( val=read_rotary() ) {
    c +=val;
    if(c > 100){
      c=100;
      } else if (c < 0){
        c=0;
        }
    Serial.print(c);Serial.print(" ");
    // draw volume percent
    drawPerc(c);
  }
}
