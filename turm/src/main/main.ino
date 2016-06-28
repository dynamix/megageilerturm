// we are going to use fast led as it provides some nice helper instead of the
// normal neopixel stuff
#include <FastLED.h>
#include <SD.h>


// bluetooth
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"



// what we want to do here (pretty sure not everything will get done in time - only 1-2 would be totally ok :-)

// a) have a simple on and off switch (could just turn off the batteries)
// b) have a auto mode which cycles thorugh the things we came up with
// c) have a simple mode switch to cycle through our different modes
//    - if in sd mode, make scene selectable + autoswitch as well
// d) be able to stream a previous captures pixel stream from the sd card
//    - we do have 360 pixels - that means we need 1080 bytes per frame - for 30 fps we need 32kb+/s read speed - sounds doable (we dont have room for buffering)
// e) be able to receive some control information via Bluetooth - this can be uses instead of the mode switch
// f) receive data from our microphone and i) influence the current animation or b) have a seperate audio controlled light mode
// g) receive some data from our color sensor and i) influence the current animation or b) have a seperate audio controlled light mode
// h) receive some data from our motion sensor i) influence the current animation or b) have a seperate motion controlled light mode
// i) have a potentiometer from the brightness controll :D

// planed setup - TODO this is invalid ... update later
// - color sensor on SCL, SDA
// - bluetooth on RX, TX
// - led data out on D12
// - audio in on D6
// - motion sensor on D6/D9
// - sd card reader via to be installed ICSP
// => leaves us with on free pin for a button or another data out for or fiber optic stride :yey:!!


// ideas for LED modes
// - only fibers - random glitter
// - only fibers - fading colors
//
// generic:
// - one runing pixel around the global with a trail .. followed by a 3x3 pixel catching up
// - falling rain from the top
// - random sparkling pixels
// - exploding pixel like rockets :D
// - lines runing around the circle .. maybe in oposit directions?!
// - color wheels + rainbows
// - knight riders
// - two colors switches in fast like a police car
// - slow and fast pulsing
// - forward + sparkling waves
// - scans - up down , left right mixing
// - rollercoaster over the globe
// - a rotating image or thing?!
// - three direction changing snakes that are R&G&B and mix on meetup
// - runing pixels /lineswith space inbeetween
// - light on but a snake brither
// - snake with a trail
// - stuff that stacks .. comes from one direction and stays on
// - rainbow with gliter
// - confetti
// - stars
// -- moving (horz or vert)
// -- one line move with them
// -- pulsing rings roatiting (fast and faster)
// -- bow from left to right
// --glimmer with snails
// -- really fast and light blink ... which takes a long time to fade awaay
// -- longer segments - on / off and moving
// -- all pixels at full - turning random pixels off


// IDEAs : https://vimeo.com/145339817

// with audio:
// - amped bars
// - snake that lights up the head driven by sound
// with movement:
// - translate stuff by some pixels up or down
// color sensor:
// - pick up color and just adjust


// PINS of final implementation
#define STRIPE_1_PIN 2
#define STRIPE_2_PIN 14
#define STRIPE_3_PIN 7
#define STRIPE_4_PIN 8
#define STRIPE_5_PIN 6
#define STRIPE_6_PIN 20
#define STRIPE_7_PIN 21
#define STRIPE_8_PIN 5

#define SD_IN_PIN 12
#define SD_OUT_PIN 11
#define SD_CS_PIN 10 // clock select for SD

#define ACCEL_SDA_PIN 18
#define ACCEL_SCL_PIN 19

#define POTENTIOMETER_PIN 22
#define MIC_PIN 23 // pin for our microphone

#define BLUETOOTH_RX_PIN 0
#define BLUETOOTH_TX_PIN 1

#define BUTTON_PIN 4


#define NUM_SPHERE_LEDS 360
#define NUM_FIBER_LEDS 7
#define NUM_TOP_LED 60
#define NUM_LEDS NUM_SPHERE_LEDS + NUM_FIBER_LEDS + NUM_TOP_LED
#define NUM_LEDS_PER_STRIP 60
#define NUM_STRIPS 7



#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define BAUD_RATE 500000
#define CMD_NEW_DATA 1

CRGB leds[NUM_LEDS];

uint16_t currentDelay = 0;

Adafruit_BluefruitLE_UART ble(Serial1, -1);

uint8_t button = 0;

uint16_t bytesInStrip = NUM_LEDS * 3;
uint16_t sphereBytes = NUM_SPHERE_LEDS * 3;

File file;



void error(const char* s) {
  Serial.println(s);
  while (1) {
    yield();
  }
}

int serialInt() {
  while (!Serial.available()) {}
  return Serial.read();
}

void clear() {
    memset(leds,0,sizeof(leds));
}

void setup() {

	FastLED.addLeds<WS2811_PORTD,8>(leds, NUM_LEDS_PER_STRIP);
    //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(32);
    FastLED.clear();
    FastLED.show();
    delay(3000); // if we fucked it up - great idea by fastled :D



    // init bluetooth
    // if ( !ble.begin(true) )    {
    //     error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    // }


	pinMode(BUTTON_PIN, INPUT_PULLUP);

  	if (!SD.begin(SD_CS_PIN)) {
    	error("sd failed at begin");
  	}

  	// // open or create file - truncate existing file.
  	file = SD.open("4.rec");
  	if (!file) {
    	error("open failed");
  	}

    // Serial.begin(BAUD_RATE);
    // while (!Serial.available()) {
    //    yield();
    // }

    // always celar the strip and set one test pixel to active

    // char b = Serial.read();
    // Serial.print("setup done, got");
    // Serial.println(b);

    // FastLED.clear();
    // leds[ 20 ] += CRGB::Red;
    // FastLED.show();


    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
}

uint8_t usePotentiometer = 1;


void checkButton() {
    int bs = 0;
	bs = digitalRead(BUTTON_PIN);
	if (bs == LOW) {
        nextMode();
	}
}


void checkPotentiometer() {
    static uint16_t potentiometer = 0;
    potentiometer = analogRead(POTENTIOMETER_PIN);
    uint8_t brightness = potentiometer / 4;
    if(usePotentiometer == 1) {
        LEDS.setBrightness(brightness);
    }
}

uint16_t fps = 0;
uint32_t readBytes = 0;



void showFps() {
	Serial.println(fps);
    // Serial.println(readBytes);
	fps = 0;
    readBytes = 0;
}


typedef void (*Modes[])();

// all the main modes we support
Modes modes =         {colorWheel,randomBluePixelsOnSphere,ambientRedCycle ,cycleSD, simpleAudio};
Modes setupForModes = {none,none,ambientRedCycleSetup,setupCycleSD,none};

// the current active main mode
uint8_t currentMode = 0;

// we have some modes how we use the audio data to modulate the colors
uint8_t audioMode = 0; // don't use audio data

// which file should we play next from the sd card?
uint8_t sdIdx = 1;

#define LAST_FILE_ON_SD 4

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextMode() {
  currentMode = (currentMode + 1) % ARRAY_SIZE(modes);
  FastLED.setBrightness(32);
  usePotentiometer = 1;
  currentDelay = 0;
  setupForModes[currentMode]();
}


// audio code
#define DC_OFFSET  0  // DC offset in mic signal
#define NOISE     100
#define SAMPLES   60  // samples for the mic buffer

#define MIN_DIST_AUDIO_LEVELS 10

int vol[SAMPLES];
int lvl       = 10;     // audio level dampend
int minLvlAvg = 0;
int maxLvlAvg = 512;
byte volumeSampleIndex = 0;
// Calculate bar height based on dynamic min/max levels (fixed point):
// height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

void audioUpdate() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  int      n, height;
  n   = analogRead(MIC_PIN);                 // Raw reading from mic
  n   = abs(n - 512 - DC_OFFSET);            // Center on zero
  n   = (n <= NOISE) ? 0 : (n - NOISE);      // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)

  vol[volumeSampleIndex] = n;                      // Save sample for dynamic leveling
  if(++volumeSampleIndex >= SAMPLES) volumeSampleIndex = 0; // Advance/rollover sample counter

  // Get volume range of prior frames
  minLvl = maxLvl = vol[0];
  for(i=1; i<SAMPLES; i++) {
    if(vol[i] < minLvl)      minLvl = vol[i];
    else if(vol[i] > maxLvl) maxLvl = vol[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if((maxLvl - minLvl) < MIN_DIST_AUDIO_LEVELS) maxLvl = minLvl + MIN_DIST_AUDIO_LEVELS;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6; // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6; // (fake rolling average)
}

void nextFrameFromSD() {
    if (file.read((char*)leds, sphereBytes) != sphereBytes) {
       // end of current annimation - switch to next
       sdIdx++;
       if(sdIdx > LAST_FILE_ON_SD) {
         sdIdx = 1;
       }
       file.close();
       char fn[10];
       sprintf(fn, "%d.rec", sdIdx);
       // we just assume this works for now :D
       file = SD.open(fn);
       // maybe we should fadeout the old scene for a smooth change?
       nextFrameFromSD();
    }
}


// Serial test code
uint32_t available = 0;
uint32_t read = 0;
uint32_t toRead = 0;
uint32_t offset = 0;

void setupCycleSD() {
    currentDelay = 100;
}

void cycleSD() {
    nextFrameFromSD();
}



// color wheel
//hue++;
void colorWheel() {
    static uint8_t hue = 0;
    for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
            leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
        }
    }
}

void randomBluePixelsOnSphere() {
    for(int i =0;i<20;i++) {
        int x = random( 0, 360 );
        leds[x] = CRGB::Blue;
    }
}

void none() {}

void ambientRedCycleSetup() {
    usePotentiometer = 0;
    FastLED.setBrightness(255);
}

void ambientRedCycle() {
    static uint8_t ambient = 0;
    leds[NUM_SPHERE_LEDS + (ambient % 7)] = CHSV(250,192,255);
    EVERY_N_SECONDS(1) { ambient++ ; }
    // leds[NUM_SPHERE_LEDS+1] = CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+2] = CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+3] = CHSV(100,120,255);
    // leds[NUM_SPHERE_LEDS+4] = CHSV(200,180,255);
    // leds[NUM_SPHERE_LEDS+5] = CHSV(220,240,255);
    // leds[NUM_SPHERE_LEDS+6] = CHSV(220,240,255);
}


#define TOP 30

void simpleAudio() {
    audioUpdate();
    int height;
    height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

    if(height < 0L)       height = 0;      // Clip output
    else if(height > TOP) height = TOP;
    // if(height > peak)     peak   = height; // Keep 'peak' dot at top
    uint8_t jj = 0;
    for(int i = 0; i < 12; i ++) {
        for(int j = 0; j < height; j ++) {
            if( i % 2 == 0) {
                jj = 30 - j;
            } else {
                jj = j;
            }
            leds[i*30+jj] = CRGB::Green;
        }
    }
}


void checkSerial() {
    if(Serial.available() > 0) {
        Serial.read();
        nextMode();
    }
}

void loop() {
    // static uint8_t hue = 0;
    // if(available = Serial.available()) {

    //     if(offset + available > 1080) {
    //         toRead = 1080 - offset;
    //     }  else {
    //         toRead = available;
    //     }

    //     // uint8_t c = Serial.read();
    //     Serial.readBytes(((char*)leds)+offset,toRead);

    //     offset = offset + toRead;

    //     if(offset == 1080) {
    //         FastLED.show();
    //         offset = 0;
    //     }

    //     readBytes = readBytes + read;
    //     // Serial.print("got ");
    //     // if (readBytes == 360) {
    //     //     Serial.print("got 1000 available=");
    //     //     Serial.println(available);
    //     //     readBytes = 0;
    //     // }

    //     // Serial.print("of ");
    //     // Serial.println(available);
    // }

    // leds[ button ] += CRGB::White;
    // leds[ pos ] += CRGB::Red;
    // FastLED.show();


    // pos++;
    // if (pos > 360) {
    //     pos = 0;
    // }

	// leds[ button ] += CRGB::White;
    // leds[ pos ] += CRGB::Red;
	// FastLED.show();

     // LEDS.clear();
     // for(int i =0;i<360+60;i++) {
     //   leds[i] = CRGB::Black;
     // }

     // FastLED.clear();
     // FastLED.showColor(CRGB::White);

     // Serial.print("a");

    //  if(button == 2) {
    //  } else if (button == 1)  {
    //     for(int i = 0; i < NUM_STRIPS-5; i++) {
    //         for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
    //             leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    //         }
    //     }
    // }
    // else if (button == 0) {

    //     // audioUpdate();
    //     // for(int i = 0; i < NUM_STRIPS; i++) {
    //     //     for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
    //     //         leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    //     //     }
    //     // }
    // } else if (button == 3){
    //     //nextFrameFromSD();
    // } else {
    //     LEDS.clear();
    //     leds[NUM_SPHERE_LEDS + (ambient % 7)] = CHSV(250,192,255);
    //     // leds[NUM_SPHERE_LEDS+1] = CHSV(hue,120,255);
    //     // leds[NUM_SPHERE_LEDS+2] = CHSV(hue,120,255);
    //     // leds[NUM_SPHERE_LEDS+3] = CHSV(100,120,255);
    //     // leds[NUM_SPHERE_LEDS+4] = CHSV(200,180,255);
    //     // leds[NUM_SPHERE_LEDS+5] = CHSV(220,240,255);
    //     // leds[NUM_SPHERE_LEDS+6] = CHSV(220,240,255);
    // }
    // // Set the first n leds on each strip to show which strip it is
    // // for(int i = 0; i < NUM_STRIPS; i++) {
    // //     for(int j = 0; j <= i; j++) {
    // //         leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
    // //     }
    // // }

    // //hue++;
    clear();
    modes[currentMode]();
    FastLED.show();

	// check if our main  button was pressed
    EVERY_N_MILLISECONDS(100) { checkPotentiometer(); }
    EVERY_N_SECONDS(1) { checkButton(); checkSerial(); }

    // FastLED.show();
    FastLED.delay(currentDelay);
}
