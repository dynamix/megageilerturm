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

#define MAX_BRIGHTNESS 32
#define MIN_BRIGHTNESS 8

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

uint8_t blueOk = 0;

void setup() {

	FastLED.addLeds<WS2811_PORTD,8>(leds, NUM_LEDS_PER_STRIP);
    //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    delay(3000); // if we fucked it up - great idea by fastled :D

    // init bluetooth
    if ( ble.begin(true) )    {
      ble.sendCommandCheckOK("AT+GAPDEVNAME=megageilerturm");
      ble.setMode(BLUEFRUIT_MODE_DATA);
      // TODO: give visible signal
    }


	pinMode(BUTTON_PIN, INPUT_PULLUP);

//  	if (!SD.begin(SD_CS_PIN)) {
  //  	error("sd failed at begin");
 // 	}

  	// // open or create file - truncate existing file.
  //	file = SD.open("4.rec");
 // 	if (!file) {
 //   	error("open failed");//
  //	}

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

// the current active main mode
int8_t currentMode = 0;
int8_t previousMode = 0;
uint8_t currentBrightness = MAX_BRIGHTNESS;


void checkButton() {
  int bs = 0;
  static uint8_t pressedFor = 0;
	bs = digitalRead(BUTTON_PIN);
	if(bs == LOW) {
      pressedFor++;
      nextMode(1);
	} else {
    pressedFor = 0;
  }
  if(pressedFor == 2) {
    currentMode = -1;
    nextMode(1);
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
Modes modes =         {
  ambientAllRainbow,
  ambientRedCycle,
  ringAudio,
  simpleAudio,
  twoRingAudio,
  colorWheel,
  fastColorWheel,
  colorWheelPulsing,
  colorWheelUpDown,
  segmentTurning,
  randomBluePixelsOnSphere,
  rainbowSparks,
  randomSparks,
  sparks,
  sparksAndRainbow,
  threeSnakes,
  lightning,
  fire
};

Modes setupForModes = {
  ambientAllRainbowSetup,
  ambientRedCycleSetup,
  ringAudioSetup,
  none,
  twoRingAudioSetup,
  none,
  none,
  none,
  colorWheelUpDownSetup,
  segmentTurningSetup,
  none,
  rainbowSparksSetup,
  randomSparksSetup,
  sparksSetup,
  sparksAndRainbowSetup,
  threeSnakesSetup,
  lightningSetup,
  fireSetup
};


// we have some modes how we use the audio data to modulate the colors
uint8_t audioMode = 0; // don't use audio data

// which file should we play next from the sd card?
uint8_t sdIdx = 1;

#define LAST_FILE_ON_SD 4

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextMode(uint8_t dir) {
  uint8_t  newMode;
  newMode = (currentMode + dir) % ARRAY_SIZE(modes);
  setMode(newMode);
}

void setMode(uint8_t mode){
  previousMode = currentMode;
  currentMode = mode;
  if(currentMode < 0)
    currentMode = ARRAY_SIZE(modes) - 1;
  setBrightness(MAX_BRIGHTNESS);
  usePotentiometer = 1;
  currentDelay = 0;
  setupForModes[currentMode]();
}

void setBrightness(uint8_t brightness){
  currentBrightness = brightness;
  if (currentBrightness > MAX_BRIGHTNESS) {
    currentBrightness = MAX_BRIGHTNESS;
  }
  if (currentBrightness < MIN_BRIGHTNESS){
    currentBrightness = MIN_BRIGHTNESS;
  }
  FastLED.setBrightness(currentBrightness);
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
    hue++;
    for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
            leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
        }
    }
}

void colorWheelPulsing() {
    static uint8_t hue = 0;
    static uint8_t pulse = 255;
    static int8_t dir = -1;

    pulse += dir;

    if(pulse < 100) {
      dir = 1;
    }
    if(pulse > 253) {
      dir = -1;
    }

    hue++;
    for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
            leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,pulse);
        }
    }
}

void colorWheelUpDownSetup() {
  currentDelay = 15;
}

void colorWheelUpDown() {
    static uint8_t hue = 0;
    static uint8_t aaa = 0;
    hue++;
    uint8_t jj = 0;
    for(int i = 0; i < 12; i++) {
        for(int j = 0; j < 30; j++) {
              if( i % 2 == 0) {
                jj = 29 - j;
            } else {
                jj = j;
            }
            leds[xy60x6(i,jj)] = CHSV((32*(j/3)) + hue,200,255);
        }
    }
}

void fastColorWheel() {
    static uint8_t hue = 0;
    hue++;
    hue++;
    hue++;
    for(int i = 0; i < NUM_STRIPS; i++) {
        for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
            leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
        }
    }
}

#define WATERFALLS 4

void waterfall() {
   // static uint8_t[] segment = {3,7,11,2};
   // static float[] pos = {0,0,0,0};
   // static float[] speed = {0.2,0.5,1,0.8};

 // for(int s = 0; s<WATERFALLS;s++) {
 //   for(int p = 0;p<5;p++) {
 //     leds[segment[s]] = CHSV(0,0,0);
  //  }
  //}




}

void waterfallSetup() {

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
    static uint8_t onBlink = 0;
    EVERY_N_MILLISECONDS(500) { onBlink++; if(onBlink>2) onBlink = 0; }
    if(onBlink) {
      leds[420] = CRGB::Red;
      leds[418] = CRGB::Red;
      leds[416] = CRGB::Red;
    }

    leds[NUM_SPHERE_LEDS + (ambient % 7)] = CHSV(250,192,255);
    EVERY_N_SECONDS(1) { ambient++ ; }
    // leds[NUM_SPHERE_LEDS+1] = CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+1] = CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+2] = CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+3] = CHSV(100,120,255);
    // leds[NUM_SPHERE_LEDS+4] = CHSV(200,180,255);
    // leds[NUM_SPHERE_LEDS+5] = CHSV(220,240,255);
    // leds[NUM_SPHERE_LEDS+6] = CHSV(220,240,255);
}

void ambientOnlyNoneSideEmiting() {
    static uint8_t hue = 0;
    hue++;
    static uint8_t onBlink = 0;
    EVERY_N_MILLISECONDS(500) { onBlink++; if(onBlink>2) onBlink = 0; }
    if(onBlink) {
      leds[420] = CRGB::Red;
      leds[418] = CRGB::Red;
      leds[416] = CRGB::Red;
    }

      // static uint8_t ambient = 0;
    // leds[NUM_SPHERE_LEDS + (ambient % 7)] = CHSV(250,192,255);
    // EVERY_N_SECONDS(1) { ambient++ ; }
    // leds[NUM_SPHERE_LEDS] = CRGB::Green; //CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+2] = CRGB::Green; //CHSV(hue,120,255);
    // leds[NUM_SPHERE_LEDS+3] = CRGB::White; //CHSV(100,120,255);
    // leds[NUM_SPHERE_LEDS+4] = CRGB::Red; //CHSV(200,180,255);
    leds[NUM_SPHERE_LEDS+5] = CHSV(hue,240,255);
    leds[NUM_SPHERE_LEDS+1] = CHSV(hue-100,240,255);

    // leds[NUM_SPHERE_LEDS+6] = CRGB::Cyan; //CHSV(220,240,255);
}

void ambientOnlyNoneSideEmitingSetup() {
    usePotentiometer = 0;
    FastLED.setBrightness(255);
}



void ambientAllRainbow() {
    static uint8_t hue = 0;
    hue++;

    static uint8_t onBlink = 0;
    EVERY_N_MILLISECONDS(500) { onBlink++; if(onBlink>2) onBlink = 0; }
    if(onBlink) {
      leds[420] = CRGB::Red;
      leds[418] = CRGB::Red;
      leds[416] = CRGB::Red;
    }

    leds[NUM_SPHERE_LEDS] = CHSV(hue+120,240,255);
    leds[NUM_SPHERE_LEDS+2] = CHSV(hue+75,240,255);
    leds[NUM_SPHERE_LEDS+3] = CHSV(hue-50,240,255);
    leds[NUM_SPHERE_LEDS+4] = CHSV(hue,240,255);
    leds[NUM_SPHERE_LEDS+5] = CHSV(hue+50,240,255);
    leds[NUM_SPHERE_LEDS+1] = CHSV(hue-100,240,255);
}

void ambientAllRainbowSetup() {
    usePotentiometer = 0;
    FastLED.setBrightness(255);
}

void ambientSparkling() {
    static uint8_t onBlink = 0;
    EVERY_N_MILLISECONDS(500) { onBlink++; if(onBlink>2) onBlink = 0; }
    if(onBlink) {
      leds[420] = CRGB::Red;
      leds[418] = CRGB::Red;
      leds[416] = CRGB::Red;
    }

    static uint8_t on = 0;
    EVERY_N_MILLISECONDS(500) { on++; if(on>2) on = 0; }
    if(on == 1 ) {
      leds[NUM_SPHERE_LEDS+5] = CRGB::Blue;
      leds[NUM_SPHERE_LEDS+1] = CRGB::White;
    }
}

void ambientSparklingSetup() {
    usePotentiometer = 0;
    FastLED.setBrightness(255);
}


#define TOP 30

void simpleAudio() {
    audioUpdate();
    int height;
    static uint8_t hue = 0;
    hue++;
    height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

    Serial.print(lvl);
    Serial.print(" ");
    Serial.print(minLvlAvg);
    Serial.print(" ");
    Serial.println(maxLvlAvg);

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
            leds[i*30+jj] = CHSV((32*i) + hue,192+10,255);;
        }
    }

    // spitye
    for(int j = 0; j < height; j ++) {
        leds[NUM_SPHERE_LEDS + NUM_FIBER_LEDS + j ] = CRGB::Red;
    }
}

void sparks() {
    for(int j = 0; j < 5; j ++) {
        leds[random( 0, 360 )] = CRGB::White;
    }
}

void sparksSetup() {
    FastLED.setBrightness(255);
    currentDelay = 10;
    usePotentiometer = 0;
}

void randomSparks() {
    for(int j = 0; j < 5; j ++) {
        leds[random( 0, 360 )] = CHSV(random(0, 255),190,255);
    }
    leds[360+7+random( 0, 50 )] = CHSV(random(0, 255),190,255);
    leds[360+7+random( 0, 50 )] = CHSV(random(0, 255),220,255);
}

void randomSparksSetup() {
    FastLED.setBrightness(255);
    currentDelay = 40;
    usePotentiometer = 0;
}


void rainbowSparks() {
    static uint8_t hue = 0;
    hue++;
    for(int j = 0; j < 5; j ++) {
        leds[random( 0, 360 )] = CHSV(hue,210,255);
    }
}

void rainbowSparksSetup() {
    FastLED.setBrightness(255);
    currentDelay = 10;
    usePotentiometer = 0;
}


void sparksAndRainbow() {
    static int8_t dir = -1;
    static int8_t pos = 15;
    // for(int j = 0; j < 2; j ++) {
    EVERY_N_MILLISECONDS(20) { leds[random( 0, 360 )] = CRGB::White; }

    EVERY_N_MILLISECONDS(50) { leds[random( 360+7, 360+7+54 )] = CRGB::Red; }
    // }

    static uint8_t hue = 0;
    hue++;

    uint8_t jj = 0;

    if(pos <= 1) {
        dir = 1;
    }
    if(pos >= 29) {
        dir = -1;
    }
    pos += dir;

    for(int i = 0; i < 12; i ++) {
        if( i % 2 == 0) {
            jj = 30 - pos;
        } else {
            jj = pos;
        }
       uint16_t p = i*30+jj;

       leds[p-1] = CHSV((32*i) + hue,192,64);
       leds[p] = CHSV((32*i) + hue,192+10,64);
       leds[p+1] = CHSV((32*i) + hue,192+20,64);
    }

}

void sparksAndRainbowSetup() {
    FastLED.setBrightness(255);
    currentDelay = 10;
    usePotentiometer = 0;
}

#define SNAKE_LENGTH 5

uint16_t xy60x6(uint8_t x, uint8_t y) {
    if(y > 29) {
        x = x + 6;
        y = 59 - y;
    }
    return (x * 30) + y;
}

void threeSnakes() {
    static uint8_t snake1 = 0;
    static uint8_t snake2 = 20;
    static uint8_t snake3 = 40;

    // for(int j = 0; j < 2; j ++) {
    // EVERY_N_MILLISECONDS(20) { leds[random( 0, 360 )] = CRGB::White; }

    EVERY_N_MILLISECONDS(50) { leds[random( 360+7, 360+7+54 )] = CRGB::Red; }
    // }

    static uint8_t hue = 0;
    hue++;

    snake1++;
    if(snake1 >= 60) {
      snake1 = 0;
    }

    snake3++;
    if(snake3 >= 60) {
      snake3 = 0;
    }

    snake2--;
    if(snake1 >= 60) {
      snake1 = 60;
    }

    for(int j = 0; j < 4; j ++) {
        for(int i = 0; i < SNAKE_LENGTH; i ++) {
           leds[ xy60x6(j,(snake1+i+j*2) % 60) ] = CHSV(100 + hue,192,170+ i*15);
           leds[ xy60x6(j,(snake2+i+j*2) % 60) ] += CHSV(50 + hue,192,170+ i*15);
           leds[ xy60x6(j,(snake3+i+j*2) % 60) ] += CHSV(hue,192,170+ i*15);
           // leds[p] = CHSV((32*i) + hue,192+10,64);
           // leds[p+1] = CHSV((32*i) + hue,192+20,64);
        }
    }

}

void threeSnakesSetup() {
    currentDelay = 100;
}

CRGBPalette16 firePal;

void fireSetup() {
  currentDelay = 60;
  firePal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::Grey);
}

#define COOLING  75
#define SPARKING 100
#define Y 30
#define X 13


void fire() {
  static byte heat[NUM_LEDS];
  random16_add_entropy(random());
  // cool down
  for (int x = 0; x < X; x++) {
      for( int y = 0; y < Y; y++) {
        heat[y+x*Y] = qsub8( heat[y+x*Y],  random8(0, ((COOLING * 10) / Y) + 2));
      }
  }
  // drift up and difuse
  for (int x = 0; x < X; x++) {
    for( int k= Y - 1; k >= 2; k--) {
      int kk = k;
      int dir = 1;
      if(x % 2 != 0) {
        kk = 29 - k;
        dir = -1;
      }
      heat[(kk+x*Y)] = (heat[(kk+x*Y) - dir] + heat[(kk+x*Y) - (2*dir)] + heat[(kk+x*Y) - (2*dir)] ) / 3;
    }
  }
  // ignite
  for(int i = 0; i < X; i++) {
    if( random8() < SPARKING ) {
      int y = random8(7);
      int x = i;
      int yy = y;
      if(x % 2 != 0)
        yy = 29 - y;
      heat[yy+x*30] = qadd8( heat[yy+x*30], random8(100,180) );
    }
  }
  // map to pixels
  for( int j = 0; j < NUM_LEDS; j++) {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( firePal, colorindex);
    leds[j] = color;
  }
}


void ringAudio() {
    audioUpdate();

    uint8_t y = 0;
    uint8_t y2 = 0;
    uint8_t ymax = 0;
    static uint8_t peak = 0;

    y = 40 * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

    if(y < 0L)       y = 0;
    else if(y > 29) {
        y = 29;
        y2 = y - 29;
        if(y2 > 40) {
            y2 = 40;
        }
    }

    if( y > peak ) {
        peak = y;
    }

    EVERY_N_MILLISECONDS(200) { if(peak >  0) peak--; }

    uint8_t jj = 0;
    for(int i = 0; i < 12; i ++) {

        for(int j = 0; j < y; j ++) {
            if( i % 2 == 0) {
                jj = y;
            } else {
                jj = 30- y;
            }
            leds[i*30+jj] = CRGB::BlueViolet;
        }

        for(int p = peak; p > y; p--) {
            if(p == peak) {
                //leds[i*30+jj+p] = CRGB::Green;
            } else {
               // leds[i*30+jj+p] = CHSV(200,200, 250 - ( (peak-p) * (150/peak) ) );
            }
        }
    }

    for(int j = 0; j < y2; j ++) {
        leds[NUM_SPHERE_LEDS + NUM_FIBER_LEDS + j ] = CRGB::BlueViolet;
    }


}
void ringAudioSetup() {
    currentDelay = 20;
}


void twoRingAudio() {
    audioUpdate();

    uint8_t y = 0;
    uint8_t y2 = 0;
    uint8_t ymax = 0;
    uint8_t peak = 0;
    static uint8_t hue = 0;

    EVERY_N_MILLISECONDS(50) { hue++; }

    y = 15 * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

    if(y < 0L)       y = 0;
    else if(y > 15) {
        y = 14;
    }


    uint8_t jj = 0;
    uint8_t m = 0;
    uint8_t n = 0;
    for(int i = 0; i < 12; i ++) {

        for(int j = -y; j < y; j++) {


            for(int z=0;z<=y;z++) {
              m = 15 + z;
              if( i % 2 == 0) {
                  jj = m;
              } else {
                  jj = 30-m;
              }
              leds[i*30+jj] = CHSV(30*z+hue,180,255);
            }

            for(int z=0;z<=y;z++) {
              m = 15 - z;

              if( i % 2 == 0) {
                  jj = m;
              } else {
                  jj = 30-m;
              }
              leds[i*30+jj] = CHSV(30*z+hue,180,255);
            }
        }

    }

}
void twoRingAudioSetup() {
    currentDelay = 20;
}


void segmentTurning() {
    static uint8_t hue = 0;
    static int8_t segment = 0;
    static int8_t segment2 = 6;
    hue++;
    segment++;
    segment2--;
    if(segment2 < 0) {
      segment2 = 11;
    }
    if(segment >= 12) {
      segment = 0;
    }
    for(uint8_t i = 0; i<30;i++) {
      leds[30*segment+i] +=  CHSV((16*i) + hue + segment  ,192+20,210);
      leds[30*segment2+i] +=  CHSV((16*i) + hue + segment2 ,192+20,210);
    }
}

void segmentTurningSetup() {
    currentDelay = 150;
}

void accel() {

}

void accelSetup() {

}




// lighning from top where the last flash ignites the whole sphere
uint8_t flashes = 8;

void lightning(){
  for (int flash = 0; flash < flashes; flash++) {
    uint8_t brightness = 255;     // the brightness of the leader is scaled down by a factor of 5
    uint8_t length = random8(1,60);
    uint8_t duration = 8;

    // last partial flash fills 
    if (flash == 0){
    }else if (flash == flashes -3) {
      length = 60;
      brightness = 255;
      duration = 10;
    } else if (flash >= flashes-3){
      // last two flashes:
      duration = 20;
      length = 120;
      brightness = 255;
    }
    lightningOfLength(length, brightness);

    delay(duration);                 // each flash only lasts 4-10 milliseconds

    lightningOfLength(length, 0);

    if (flash == 0) delay (150);   // longer delay until next flash after the leader
    delay(50+random8(50));               // shorter delay between strokes
  }
  delay(500);               // shorter delay between strokes
  setMode(previousMode);
}

void lightningSetup(){
}

void lightningOfLength(uint8_t length, uint8_t brightness) {
  if (length <= 60){
    // fill part of the top
    for (int i =1; i < length; i++){
      leds[NUM_LEDS - i] = CHSV(255, 0, brightness);
    }
  }else{
    // fill every pixel
    for(int i = 0; i < NUM_LEDS; i ++) {
            leds[i] = CHSV(255, 0, brightness);
    }
  }
  FastLED.show();                       // Show a section of LED's
}

void checkSerial() {
    if(Serial.available() > 0) {
        Serial.read();
        nextMode(1);
    }
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);

// the packet buffer
extern uint8_t packetbuffer[];

void readBT() {
  uint8_t len = readPacket(&ble, 10);
  if (len == 0) return;

    // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';

    // button released
    if(!pressed){
      // UP & DOWN arrows -> brightness
      if(buttnum == 6) {
        setBrightness(currentBrightness + 8);
      }
      if(buttnum == 5) {
        setBrightness(currentBrightness - 8);
      }
      // SIDE BUTTONS -> mode switch
      if(buttnum == 7) {
        nextMode(-1);
      }
      if(buttnum == 8) {
        nextMode(1);
      }

      // direct modes:
      if(buttnum == 1) {
        setMode(0);
      }
      if(buttnum == 2) {
        setMode(4);
      }
      if(buttnum == 3) {
        setMode(10);
      }
      if(buttnum == 4) {
        setMode(16);
      }
    }
    // Serial.print ("Button "); Serial.print(buttnum);
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
    EVERY_N_SECONDS(1) { checkSerial(); }
    EVERY_N_MILLISECONDS(500) { checkButton(); readBT(); }



    // FastLED.show();
    FastLED.delay(currentDelay);
}
