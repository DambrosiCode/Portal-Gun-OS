/*
  Portal Gun V3.1 © 2026 by dambrosicode is licensed under CC BY-NC 4.0
*/

// Includes:
#include <Arduino.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>
#include <RotaryEncoder.h>
#include "configuration.cpp"
#include <ezButton.h>  // the library to use for SW pin
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
hw_timer_t *smTimer = nullptr;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                           INITIALIZATION                                           //////////

// Class for a LED Color:
class ColorHSV {
public:
  uint8_t h = 0;
  uint8_t s = 0;
  uint8_t v = 0;
  ColorHSV(uint8_t h, uint8_t s, uint8_t v) : h(h), s(s), v(v) {};
};

// Struct containing all LED data:
struct LEDData {
  ColorHSV color_1        = {COL1_H, COL1_S, COL1_V};  // Orange
  ColorHSV color_2        = {COL2_H, COL2_S, COL2_V};  // Blue
  ColorHSV color_current  = {COL0_H, COL0_S, COL0_V};  // Current Color
  ColorHSV color_settings = {COL3_H, COL3_S, COL3_V};  // Settings Color

  float currentBrightness = 0.0;

  CRGB mainArray[LED_COUNT_MAIN];
  CRGB clawArray[LED_COUNT_CLAW];
};
LEDData leds; // Global LED Data

// Struct containing all Sound data:
#if ENABLE_SOUND
  EspSoftwareSerial::UART SoftSerial1(PIN_SPEAKER1_RX, PIN_SPEAKER1_TX);
  EspSoftwareSerial::UART SoftSerial2(PIN_SPEAKER2_RX, PIN_SPEAKER2_TX);
  DFPlayerMini_Fast dfPlayer1; // Idle-Humm     (5W) 
  DFPlayerMini_Fast dfPlayer2; // Sound-Effects (3W)
  uint8_t volume_spk1 = VOLUME_PLAYER1;
  uint8_t volume_spk2 = VOLUME_PLAYER2;
#endif

enum SOUND_IDLE_FILE{   // Soundfiles for Speaker 1
  SOUND0_IDLE = 1
};

enum SOUND_EFFECT_FILE{ // Soundfiles for Speaker 2
  SOUND1_ORANGE_1      = 1,
  SOUND1_ORANGE_2      = 2,
  SOUND1_ORANGE_3      = 3,
  SOUND1_ORANGE_4      = 4,
  SOUND1_BLUE_1        = 5,
  SOUND1_BLUE_2        = 6,
  SOUND1_BLUE_3        = 7,
  SOUND1_BLUE_4        = 8
};

// Flickering Variables:
#if ENABLE_LED_FLICKERING
struct flickerData
{
  uint8_t step = (MAX_BRIGHTNESS + MIN_BRIGHTNESS) / 2;
  uint8_t max = MAX_BRIGHTNESS;
  uint8_t min = MIN_BRIGHTNESS;
};
flickerData flicker;
#endif

#define NUM_FLASH_TICKS (FLASH_DURATION / (SM_TICK_RATE / 1000))  // Number of ticks for the entire flash duration
#define FLASH_TICK_DROP NUM_FLASH_TICKS / 2                       // Number of ticks for full brightness. Brightn. then drops to 0 and rebuilds to MIN_BRIGHTNESS
#define FLASH_REBUILD_SLOPE float(((MIN_BRIGHTNESS - FLASH_DROP) / (NUM_FLASH_TICKS - FLASH_TICK_DROP)))  // Slope for rebuilding brightness

// Battery Data:
struct batteryData
{
  uint16_t allReadings[20];
  uint8_t readingIndex = 0;
  uint16_t currentReading = 0;
  uint16_t sumReadings = 0;
  uint16_t averageReading = 0;
  float percentage = 0.0;
};
batteryData battery1;
batteryData battery2;

// Rotary Encoder (Will be created in setupPins()):
RotaryEncoder *rotaryEnc = nullptr;

// State machine states:
enum SM_STATE
{
  IDLE,
  SHOOTING1,
  SHOOTING2
};
SM_STATE sm_state = IDLE;

// Variables and Flags:
volatile bool flagStateMachineTickIsDue = false;
volatile bool flagOrangePressed = false;
volatile bool flagBluePressed = false;
volatile bool flagREleft = false;
volatile bool flagREright = false;
volatile bool flagREpress = false;


//////////                                           INITIALIZATION                                           //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                         FUNCTION PROTOTYPES                                        //////////

// Setup Functions:
bool setupSerial();
bool setupLEDs();
bool setupPins();
bool setupSMTimer();
bool setupSound();

// Button press ISR Functions:
void IRAM_ATTR onOrangeButtonPress();
void IRAM_ATTR onBlueButtonPress();
void IRAM_ATTR onRotaryEncoderRotation();
void IRAM_ATTR onRotaryEncoderPress();

// State Machine Functions:
bool IRAM_ATTR flagStateMachineTick(void* timerNo);   // Flag indicating readyness for state machine tick
bool IRAM_ATTR     stateMachineTick();                // Execute Machine Tick
void stateIdle();                                     // Idle State
void stateShooting1();                                // Shooting State for Orange Portal
void stateShooting2();                                // Shooting State for Blue Portal

// Other Functions:
void Serial_printLogo();
void readAndIndicateBattery();
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

//////////                                         FUNCTION PROTOTYPES                                        //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                                SETUP                                               //////////

void setup()
{
  // Setup Serial Communication:
  setupSerial();

  // Setup Sound:
  #if ENABLE_SOUND
    setupSound();
  #endif

  // Setup LEDs:
  setupLEDs();

  // Setup Pins:
  setupPins();

  // Setup State Machine Timer:
  setupSMTimer();

  // Setup Rotary
  setupRotary();
  
  // Setup Display
  setupDisplay();
  
  debugPrint("> Setup Complete. Starting State Machine @ ");
  debugPrint(SM_TICK_RATE / 1000);
  debugPrintln("ms\n//////////////////////////////////////////////");
  Serial.flush();
} 

// Setup Function for Serial Communication:
bool setupSerial()
{
  // Setup Serial and print startup message:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("+------------------------------------------------------------------+");
  Serial.println("|  Portal Gun V3.1 © 2026 by dambrosicode is licensed under CC BY-NC 4.0  |");
  Serial.println("+------------------------------------------------------------------+\n");
  Serial_printLogo();

  Serial.print("Starting Portal Gun V3.1 on '");
  Serial.print(ARDUINO_BOARD);
  Serial.print("' @ ");
  Serial.print(F_CPU / 1000000);
  Serial.println("MHz\n");
  
  return true;
}

// Setup Function for LEDs:
bool setupLEDs()
{
  // Setup Main and Claw LEDs:
  debugPrint("> Setup LEDs... ");
  FastLED.addLeds<WS2812B, PIN_LED_MAIN, GRB>(leds.mainArray, LED_COUNT_MAIN);
  
  // Set all LEDs to the initial color:
  fill_solid(leds.mainArray, LED_COUNT_MAIN, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v));
  FastLED.show();
  delay(10);
  
  debugPrintln("Done.");
  return true;
}

bool setupPins()
{
  debugPrint("> Setup Pins... ");
  // Input Buttons:
  pinMode(PIN_BUTTON_ORANGE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_BLUE,   INPUT_PULLUP);
  pinMode(PIN_ROTENC_CLK,    INPUT_PULLUP);
  pinMode(PIN_ROTENC_DT,     INPUT_PULLUP);
  pinMode(PIN_ROTENC_SW,     INPUT_PULLUP);

  // Setup Button Interrupts:
  attachInterrupt(PIN_BUTTON_ORANGE, onOrangeButtonPress, FALLING);
  attachInterrupt(PIN_BUTTON_BLUE,   onBlueButtonPress,   FALLING);
  
  // Setup Rotary Encoder:
  rotaryEnc = new RotaryEncoder(PIN_ROTENC_DT, PIN_ROTENC_CLK, RotaryEncoder::LatchMode::FOUR0);
  attachInterrupt(PIN_ROTENC_CLK,    onRotaryEncoderRotation, CHANGE);
  attachInterrupt(PIN_ROTENC_DT,     onRotaryEncoderRotation, CHANGE);
  attachInterrupt(PIN_ROTENC_SW,     onRotaryEncoderPress,    FALLING);

  debugPrintln("Done.");
  return true;
}

// setup rotary
bool setupRotary(){
    debugPrint("> Setup Rotary... ");
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  
  return true;
}


// setup oled display
bool setupDisplay(){
    debugPrint(">Setup Display... ");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  oled_volumeDisplay(volume_spk1);
  
  return true;
}

// State Machine Functions:
void IRAM_ATTR onSMTimer()
{
  flagStateMachineTickIsDue = true;
}
bool setupSMTimer()
{
  // Create timer running at 1 MHz (1 tick = 1 microsecond)
  smTimer = timerBegin(1000000);

  if (!smTimer)
  {
    debugPrintln("> Failed to create timer.");
    return false;
  }

  timerAttachInterrupt(smTimer, &onSMTimer);
  timerAlarm(smTimer, SM_TICK_RATE, true, 0);  // microseconds, autoreload

  debugPrint("> Setup State Machine Timer @ ");
  debugPrint(SM_TICK_RATE);
  debugPrint("us (");
  debugPrint(1000000 / SM_TICK_RATE);
  debugPrintln("Hz)");

  return true;
}

#if ENABLE_SOUND
bool setupSound()
{
  // Setup Sound:
  debugPrintln("> Setup Sound... ");

  SoftSerial1.begin(9600);
  SoftSerial2.begin(9600);
  dfPlayer1.begin(SoftSerial1);
  dfPlayer2.begin(SoftSerial2);

  delay(1000);

  dfPlayer1.stop();
  dfPlayer2.stop();

  dfPlayer1.reset();
  dfPlayer2.reset();

  // Attempt to play Idle Sound:
  uint8_t player1_attempts = 0;
  while(dfPlayer1.currentFlashTrack() != SOUND_IDLE_FILE::SOUND0_IDLE && dfPlayer1.currentMode() != 2)
  {
    dfPlayer1.loop(SOUND_IDLE_FILE::SOUND0_IDLE);
    delay(150);
    player1_attempts++;
    if(player1_attempts > 10)
    {
      debugPrintln(">   Error: Player 1 not responding.");
      break;
    }
  }

  dfPlayer1.volume(volume_spk1);
  delay(200); 
  dfPlayer2.volume(volume_spk2);
  delay(200);

  debugPrint(">   Player 1 Tracks: ");
  debugPrintln(dfPlayer1.numSdTracks());
  debugPrint(">   Player 2 Tracks: ");
  debugPrintln(dfPlayer2.numSdTracks());

  debugPrintln("> Done.");

  return true;
}
#endif

//////////                                                SETUP                                               //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                                LOOP                                                //////////

void loop()
{  
  // Run State Machine:
  if(flagStateMachineTickIsDue)
  {
    stateMachineTick();
  }
  else
  {
    delay(1);
  }
}

//////////                                                LOOP                                                //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                        FUNCTION DEFINITIONS                                        //////////

// ISR Function for Orange Button Press:
void IRAM_ATTR onOrangeButtonPress()
{
  flagOrangePressed = true;
}

// ISR Function for Blue Button Press:
void IRAM_ATTR onBlueButtonPress()
{
  flagBluePressed = true;
}

void IRAM_ATTR onRotaryEncoderRotation()
{
  rotaryEnc->tick();
}

void IRAM_ATTR onRotaryEncoderPress()
{
  // Debounce:
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastPressTime < 300) {
    return;
  }
  lastPressTime = currentTime;

  flagREpress = true;
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

//Machine Tick
bool IRAM_ATTR stateMachineTick()
{
  // Get rotary Encoder Position:
  rotaryEnc->tick();
  static int8_t rot_c,val;
  //Set Volume and Volume Display
   if( val=read_rotary() ) {
      rot_c +=val;
      if(rot_c > 30){
        rot_c=30;
        } else if (rot_c < 0){
          rot_c=0;
          }
      // display
      oled_volumeDisplay(rot_c);
      // set volume
      volume_spk1 = rot_c;
      volume_spk2 = rot_c;
      dfPlayer1.volume(rot_c);
      dfPlayer2.volume(rot_c);
   }


  switch(sm_state)
  {
    case IDLE:
      stateIdle();
      break;
    case SHOOTING1:
      stateShooting1();
      break;
    case SHOOTING2:
      stateShooting2();
      break;
    default:
      break;
  }


  // Reset Flags:
  flagStateMachineTickIsDue = false;
  flagOrangePressed = false;
  flagBluePressed = false;
  flagREleft = false;
  flagREright = false;
  flagREpress = false;

  // Read Battery:
  readAndIndicateBattery();

  // Debug Interval measurement:
  #if 0
    static long tickTimer = 0;
    long tickLength = micros() - tickTimer;
    tickTimer = micros();
    debugPrint("Tick Intervall: ");
    debugPrint(tickLength);
    debugPrintln("us");
  #endif

  return true;
}

void stateIdle()
{
  // Check for Shot-button press:
  if(flagOrangePressed)    sm_state = SHOOTING1;
  else if(flagBluePressed) sm_state = SHOOTING2;

  // Update Flickering brightness:
  #if ENABLE_LED_FLICKERING
    flicker.step += random(-FLICKER_STRENGTH + 1, FLICKER_STRENGTH);
    flicker.step = constrain(flicker.step, flicker.min, flicker.max);
    leds.currentBrightness = flicker.step / 255.0;
  #endif

  // Update LED brightness:
  fill_solid(leds.mainArray, LED_COUNT_MAIN, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v * leds.currentBrightness));
  fill_solid(leds.clawArray, LED_COUNT_CLAW, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v));
  FastLED.show();
}

// Gets executed when the Orange Button is pressed. Switches Color, plays sound and returns to idle state:
void stateShooting1()
{
  static uint16_t flashTick = 0;

  if(flashTick == 0) 
  {
    debugPrintln("> Shot Portal: Orange");

    // Play random shooting sound:
    #if ENABLE_SOUND    
      int sfx = random(SOUND1_ORANGE_1, SOUND1_ORANGE_4+1);
      debugPrintln("playing: ");
      debugPrintln(sfx);
      dfPlayer2.play(sfx);
      delay(200);

      // Ensure proper volume:
      dfPlayer1.volume(volume_spk1);
      dfPlayer2.volume(volume_spk2);
    #endif
    flashTick++;
  }

  if(flashTick < FLASH_TICK_DROP)
  {
    // Full LED Brightness (Bright Flash):
    leds.currentBrightness = 1.0;
    flashTick++;
  }
  else if(flashTick == FLASH_TICK_DROP)
  {
    // Drop brightness to 0:
    leds.currentBrightness = 0.0;
    flashTick++;
  }
  else if(flashTick < NUM_FLASH_TICKS)
  {
    // Rebuild brightness:
    leds.currentBrightness += FLASH_REBUILD_SLOPE / 255.0;
    flashTick++;
  }
  else
  {
    flashTick = 0;

    // Return to IDLE:
    sm_state = IDLE;
  }

  // Update LED Color:
  leds.color_current = leds.color_1;
  fill_solid(leds.mainArray, LED_COUNT_MAIN, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v * leds.currentBrightness));
  fill_solid(leds.clawArray, LED_COUNT_CLAW, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v));
  FastLED.show();
}

void stateShooting2()
{
  // Perform Flashing:
  static uint16_t flashTick = 0;
  if(flashTick == 0) 
  {
    debugPrintln("> Shot Portal: Blue");

    // Play random shooting sound:
    #if ENABLE_SOUND
      int sfx = random(SOUND1_BLUE_1, SOUND1_BLUE_4+1);
      debugPrintln("playing: ");
      debugPrintln(sfx);
      dfPlayer2.play(sfx);
      delay(200);

      // Ensure proper volume:
      dfPlayer1.volume(volume_spk1);
      dfPlayer2.volume(volume_spk2);
    #endif
    flashTick++;
  }

  if(flashTick < FLASH_TICK_DROP)
  {
    // Full LED Brightness (Bright Flash):
    leds.currentBrightness = 1.0;
    flashTick++;
  }
  else if(flashTick == FLASH_TICK_DROP)
  {
    // Drop brightness to 0:
    leds.currentBrightness = 0.0;
    flashTick++;
  }
  else if(flashTick < NUM_FLASH_TICKS)
  {
    // Rebuild brightness:
    leds.currentBrightness += FLASH_REBUILD_SLOPE / 255.0;
    flashTick++;
  }
  else
  {
    flashTick = 0;

    // Return to IDLE:
    sm_state = IDLE;
  }

  // Update LED Color:
  leds.color_current = leds.color_2;
  fill_solid(leds.mainArray, LED_COUNT_MAIN, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v * leds.currentBrightness));
  fill_solid(leds.clawArray, LED_COUNT_CLAW, CHSV(leds.color_current.h, leds.color_current.s, leds.color_current.v));
  FastLED.show();
}

// Function for printing the Portal Gun Logo:
void Serial_printLogo()
{
  // Taken from https://emojicombos.com/portal-ascii-art
  static const char logo[] =         
"  ⠀⠀⠀⠀⢀⡠⣤⣶⣶⣶⣦⡄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⢀⣴⣿⣿⣶⣝⡻⣿⣿⣇⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⣠⡿⠿⢿⣛⠛⠋⠉⠈⠙⠿⢸⣿⣿⡷⣀⣀⠀⠀⠀⢀⣀⣀⣀⡀⠀⠀⣀⣀⣀⣀⡀⠀⣀⣀⣀⣀⠀⢀⣀⣀⣀⣀⣀⠀⣀⣀⠀⠀⣀⡀⠀⢀⣀⣀⣀⡀⠀⠀⣀⣀⣀⣀⡀\n"
"⠀⣶⣿⣿⡿⠁⠀⠀⠀⠀⠀⠀⠘⣿⢟⣼⣿⣿⡆⠀⠀⣾⣿⠛⣻⣿⠀⢸⣿⠟⠛⠛⠀⢸⣿⠟⢻⣿⡇⠘⠛⣿⣿⠛⠛⢀⣿⡟⠀⢰⣿⡇⠀⣿⡿⠛⣿⣿⠀⢰⣿⠟⠛⠛⠀\n"
"⠠⣿⣿⣟⡆⠀⠀⠀⠀⠀⠀⠀⠀⢋⣾⣿⣬⣿⣇⠀⢠⣿⡿⠿⠿⠋⠀⣾⡿⠛⠛⠇⠀⣾⣿⠿⣿⣯⠀⠀⢠⣿⡏⠀⠀⢸⣿⠃⠀⣼⣿⠀⢰⣿⡿⢿⣿⡅⠀⣾⣿⠛⠛⠇⠀\n"
"⠀⣿⢯⣾⣿⠀⠀⠀⠀⠀⠀⠀⠠⠿⠟⠉⠉⠿⠿⠀⠸⠿⠃⠀⠀⠀⠰⠿⠿⠿⠿⠃⠠⠿⠇⠠⠿⠯⠀⠀⠸⠿⠁⠀⠀⠘⠿⣷⡾⠿⠃⠀⠼⠿⠀⠸⠿⠅⠠⠿⠿⠿⠿⠃⠀\n"
"⠀⠀⣿⣿⣿⡼⣦⣄⢀⣀⣤⣤⣴⣶⣶⡿⠁⠀⡆⠀⠀⢀⣶⡄⠀⢰⣖⣲⠀⢠⡖⠒⡆⠀⢰⣖⡦⠀⢀⣴⡄⠀⠒⣶⠒⠀⣰⠖⢲⠀⠀⣶⣲⠄⠀⡆⠀⢰⣒⡂⠀⣶⣒⡆⠀\n"
"⠀⠀⠈⠻⣿⡇⣿⣿⣷⣮⣻⢿⣿⡿⠏⠀⠀⠀⠓⠲⠀⠞⠉⠹⠀⠘⠓⠚⠀⠈⠳⠖⠃⠀⠸⠀⠗⠀⠞⠉⠹⠂⠀⠻⠀⠀⠘⠲⠞⠀⠀⠇⠘⠆⠀⠇⠀⠘⠒⠆⠀⠓⠶⠃⠀\n"
"⠀⠀⠀⠀⠈⠙⠘⠿⠿⠿⠟⠓⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   \n";
  Serial.println(logo);
}

void readAndIndicateBattery()
{
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//////////                                        FUNCTION DEFINITIONS                                        //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
