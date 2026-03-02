////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                            Feature Toggles                                         //////////
#define ENABLE_DEBUG_COMMS    1  // Enable debug communication via USB with Arduino IDE (Slows down Everything. Not needed for normal operation)
#define ENABLE_LED_FLICKERING 1  // Enable LED Flickering (Brownian Motion)
#define ENABLE_SOUND          1  // Enable Sound. Disable this if you are not using any speakers.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                           General Settings                                         //////////

#define VOLUME_PLAYER1  20   // Volume for DFPlayer2 (Sound Effects)
#define VOLUME_PLAYER2  5  // Volume for DFPlayer1 (Idle-Humm)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                            LED Settings                                            //////////

// Color 0 [0, 255]: Initial Color, no Portals yet. Doubles as Low-Battery-Color for Top Indicator:
#define COL0_H  0
#define COL0_S  255
#define COL0_V  50

// Color 1 [0, 255]: Orange
#define COL1_H  13
#define COL1_S  255
#define COL1_V  255

// Color 2 [0, 255]: Blue
#define COL2_H  145
#define COL2_S  255
#define COL2_V  255

// Color 3 [0, 255]: Indication Color for settings
#define COL3_H  0
#define COL3_S  0
#define COL3_V  255

// Shooting Flash duration:
#define FLASH_DURATION 300  // [ms]
#define FLASH_DROP     MIN_BRIGHTNESS / 2   // [0, 255] Brightness drop after full brightness

// Number of LEDs per strip:
#define LED_COUNT_MAIN  22  // Number of logically connected main WS2812B LEDs (Indicator, Beam, Nozzle)
#define LED_COUNT_CLAW  2   // 3x 2 LEDs in series  (Claws)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                                Pins                                                //////////

// Pins connected to LEDs:
#define PIN_LED_MAIN     19   // Data pin for main LEDs

// Pins connected to Buttons:
#define PIN_BUTTON_ORANGE 25   // Orange button
#define PIN_BUTTON_BLUE   33   // Blue button
#define PIN_ROTENC_CLK    34   // Rotary Encoder Clock
#define PIN_ROTENC_DT     35   // Rotary Encoder Data
#define PIN_ROTENC_SW     32   // Rotary Encoder Switch

// Pins connected to Sound cards (DFPlayer Mini):
#define PIN_SPEAKER2_RX 17 // RX Pin for DFplayer1 (Sound Effects)
#define PIN_SPEAKER2_TX 16 // TX Pin for DFplayer1 (Sound Effects)
#define PIN_SPEAKER1_RX 26 // RX Pin for DFplayer2 (Idle-Humm)
#define PIN_SPEAKER1_TX 27 // TX Pin for DFplayer2 (Idle-Humm)

// pins for rotary encoder
#define CLK 34
#define DATA 35
#define SW_PIN 32


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                          Flickering Settings                                       //////////

#define FLICKER_STRENGTH 7   // [0, 255] Strength of flickering effect
#define MIN_BRIGHTNESS   150 // [0, 255] Flickering wont go below
#define MAX_BRIGHTNESS   230 // [0, 255] Flickering wont go above

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                       State-Machine Settings                                       //////////

#define SM_TICK_RATE 5000            // State machine Tick-Duration in [us] (Inverse of Frequency)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                      Battery Reading Settings                                      //////////

#define BAT_READING_SKIP 100   // Number of ticks to skip before reading battery voltage
#define BAT_ADC_MAX      1100  // ADC value for 100%    battery    (4.2V)   -> Max value
#define BAT_FULL         100   // ADC value for 'full'  battery    (4.0V)   -> Considered Full
#define BAT_LOW          760   // ADC value for 'empty' battery    (3.2V)   -> Considered Low
#define BAT_ADC_MIN      740   // ADC value for 0%      battery    (3.1V)   -> Considered Critical

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                      Rotary Settings                                               //////////

#define DIRECTION_CW 0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                      oled Display Settings                                         //////////

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                               Preprocessor debug stuff (DON'T TOUCH)                               //////////

#if ENABLE_DEBUG_COMMS
#define debugPrint(...) Serial.print(__VA_ARGS__)
#define debugPrintln(...) Serial.println(__VA_ARGS__)
#else
#define debugPrint(...)
#define debugPrintln(...)
#endif
