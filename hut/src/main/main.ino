#include <FastLED.h>

#define BUTTON_PIN 12

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define MAX_BRIGHTNESS 32
#define MIN_BRIGHTNESS 8

#define NUM_LEDS_STRIP 39
#define NUM_LEDS_RING 16

CRGB ledsRing[NUM_LEDS_RING];
CRGB ledsStrip[NUM_LEDS_STRIP];

uint16_t currentDelay = 0;
uint8_t button = 0;
uint8_t shouldClear = 1;

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
    memset(ledsRing,0,sizeof(ledsRing));
    memset(ledsStrip,0,sizeof(ledsStrip));
}
void clearnStrip(uint16_t n) {
    memset(ledsStrip,0,n*3);
}
void clearnRing(uint16_t n) {
    memset(ledsRing,0,n*3);
}

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
  dots,
  flashy,
  juggle,
  bpm,
  sinelon,
  colorWheelWithSparks,
  colorWheelPulsing,
  randomSparks,
  rainbowSparks,
  colorWheel,
  fire
};

Modes setupForModes = {
  dotsSetup,
  flashySetup,
  noclearSetup,
  noclearSetup,
  noclearSetup,
  colorWheelWithSparksSetup,
  none,
  randomSparksSetup,
  rainbowSparksSetup,
  none,
  fireSetup
};

void setup() {

  FastLED.addLeds<NEOPIXEL, 6>(ledsRing, NUM_LEDS_RING);
  FastLED.addLeds<NEOPIXEL, 9>(ledsStrip, NUM_LEDS_STRIP);

  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  delay(3000); // if we fucked it up - great idea by fastled :D

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
  setupForModes[0]();
}


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
  currentDelay = 0;
  shouldClear = 1;
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

// color wheel
void colorWheel() {
    static uint8_t hue = 0;
    hue++;
    for(int j = 0; j < NUM_LEDS_RING; j++) {
        ledsRing[j] = CHSV(16 * j + hue,192,255);
    }
    for(int j = 0; j < NUM_LEDS_STRIP; j++) {
        ledsStrip[j] = CHSV(16 * j + hue,192,255);
    }
}

void colorWheelWithSparksSetup() {
  currentDelay = 15;
  FastLED.setBrightness(255);
}


void colorWheelWithSparks() {
    static uint8_t hue = 0;
    hue++;
    for(int j = 0; j < NUM_LEDS_RING; j++) {
        ledsRing[j] = CHSV(16 * j + hue,192,64);
    }
    for(int j = 0; j < NUM_LEDS_STRIP; j++) {
        ledsStrip[j] = CHSV(16 * j + hue,192,64);
    }
    int r = random( 0, NUM_LEDS_RING+NUM_LEDS_STRIP );
    if( r >= NUM_LEDS_RING )
      ledsStrip[r - NUM_LEDS_RING] = CRGB::White;
    else
      ledsRing[r] = CRGB::White;
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
    for(int j = 0; j < NUM_LEDS_RING; j++) {
        ledsRing[j] = CHSV(16 * j + hue,192,pulse);
    }
    for(int j = 0; j < NUM_LEDS_STRIP; j++) {
        ledsStrip[j] = CHSV(16 * j + hue,192,pulse);
    }
}

void colorWheelUpDownSetup() {
  currentDelay = 15;
}

void colorWheelUpDown() {
    // static uint8_t hue = 0;
    // static uint8_t aaa = 0;
    // hue++;
    // uint8_t jj = 0;
    // for(int i = 0; i < 12; i++) {
    //     for(int j = 0; j < 30; j++) {
    //           if( i % 2 == 0) {
    //             jj = 29 - j;
    //         } else {
    //             jj = j;
    //         }
    //         leds[xy60x6(i,jj)] = CHSV((32*(j/3)) + hue,200,255);
    //     }
    // }
}

void fastColorWheel() {
    // static uint8_t hue = 0;
    // hue++;
    // hue++;
    // hue++;
    // for(int i = 0; i < NUM_STRIPS; i++) {
    //     for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
    //         leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    //     }
    // }
}

void randomBluePixelsOnSphere() {
    // for(int i =0;i<20;i++) {
    //     int x = random( 0, 360 );
    //     leds[x] = CRGB::Blue;
    // }
}

void none() {}


void sparks() {
    // for(int j = 0; j < 5; j ++) {
    //     leds[random( 0, 360 )] = CRGB::White;
    // }
}

void sparksSetup() {
    // FastLED.setBrightness(255);
    // currentDelay = 10;
}

void randomSparks() {
    ledsRing[random( 0, NUM_LEDS_RING )] = CHSV(random(0, 255),190,255);
    ledsStrip[random( 0, NUM_LEDS_STRIP )] = CHSV(random(0, 255),190,255);
}

void randomSparksSetup() {
    FastLED.setBrightness(255);
    currentDelay = 40;
}


void rainbowSparks() {
    static uint8_t hue = 0;
    hue++;
    ledsRing[random( 0, NUM_LEDS_RING )] = CHSV(hue,210,255);

    for(int j = 0; j < 5; j ++) {
      ledsStrip[random( 0, NUM_LEDS_STRIP )] = CHSV(hue,210,255);
    }
}

void rainbowSparksSetup() {
    FastLED.setBrightness(255);
    currentDelay = 10;
}


void sparksAndRainbow() {
    // static int8_t dir = -1;
    // static int8_t pos = 15;
    // // for(int j = 0; j < 2; j ++) {
    // EVERY_N_MILLISECONDS(20) { leds[random( 0, 360 )] = CRGB::White; }

    // EVERY_N_MILLISECONDS(50) { leds[random( 360+7, 360+7+54 )] = CRGB::Red; }
    // // }

    // static uint8_t hue = 0;
    // hue++;

    // uint8_t jj = 0;

    // if(pos <= 1) {
    //     dir = 1;
    // }
    // if(pos >= 29) {
    //     dir = -1;
    // }
    // pos += dir;

    // for(int i = 0; i < 12; i ++) {
    //     if( i % 2 == 0) {
    //         jj = 30 - pos;
    //     } else {
    //         jj = pos;
    //     }
    //    uint16_t p = i*30+jj;

    //    leds[p-1] = CHSV((32*i) + hue,192,64);
    //    leds[p] = CHSV((32*i) + hue,192+10,64);
    //    leds[p+1] = CHSV((32*i) + hue,192+20,64);
    // }

}

void sparksAndRainbowSetup() {
    // FastLED.setBrightness(255);
    // currentDelay = 10;
}

#define SNAKE_LENGTH 5

uint16_t xy60x6(uint8_t x, uint8_t y) {
    if(y > 29) {
        x = x + 6;
        y = 59 - y;
    }
    return (x * 30) + y;
}

uint16_t xy(uint8_t x, uint8_t y) {
  if(x%2 !=0)
    return (x%12)*30 + (y%30);
  else
    return (x%12)*30 + (29-(y%30));
}


void threeSnakes() {
    // static uint8_t snake1 = 0;
    // static uint8_t snake2 = 20;
    // static uint8_t snake3 = 40;

    // // for(int j = 0; j < 2; j ++) {
    // // EVERY_N_MILLISECONDS(20) { leds[random( 0, 360 )] = CRGB::White; }

    // EVERY_N_MILLISECONDS(50) { leds[random( 360+7, 360+7+54 )] = CRGB::Red; }
    // // }

    // static uint8_t hue = 0;
    // hue++;

    // snake1++;
    // if(snake1 >= 60) {
    //   snake1 = 0;
    // }

    // snake3++;
    // if(snake3 >= 60) {
    //   snake3 = 0;
    // }

    // snake2--;
    // if(snake1 >= 60) {
    //   snake1 = 60;
    // }

    // for(int j = 0; j < 4; j ++) {
    //     for(int i = 0; i < SNAKE_LENGTH; i ++) {
    //        leds[ xy60x6(j,(snake1+i+j*2) % 60) ] = CHSV(100 + hue,192,170+ i*15);
    //        leds[ xy60x6(j,(snake2+i+j*2) % 60) ] += CHSV(50 + hue,192,170+ i*15);
    //        leds[ xy60x6(j,(snake3+i+j*2) % 60) ] += CHSV(hue,192,170+ i*15);
    //        // leds[p] = CHSV((32*i) + hue,192+10,64);
    //        // leds[p+1] = CHSV((32*i) + hue,192+20,64);
    //     }
    // }

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
#define Y 8
#define X 2


void fire() {
  static byte heat[NUM_LEDS_RING];
  random16_add_entropy(random());

  // cool down
  for( int i = 0; i < NUM_LEDS_RING; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS_RING) + 2));
  }

  // drift up and difuse
  for( int k = Y - 1; k >= 2; k--) {
    heat[k] = ( heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  heat[1] = heat[0];

  for( int k = Y; k < (Y*2)-2; k++) {
    heat[k] = ( heat[k + 1] + heat[k + 2] + heat[k + 3] ) / 3;
  }

  heat[15] = heat[0];


  // ignite
  if( random8() < SPARKING ) {
    int y = random8(3);
    heat[y] = qadd8( heat[y], random8(100,180) );
  }

  if( random8() < SPARKING ) {
    int y = random8(2);
    heat[(Y*2)-(y+1)] = qadd8( heat[(Y*2)-y], random8(100,180) );
  }


  // map to pixels
  for( int j = 0; j < NUM_LEDS_RING; j++) {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( firePal, colorindex);
    ledsRing[(j+2)%NUM_LEDS_RING] = color;
  }
}



void sinelon() {
  static uint8_t gHue = 0;
  fadeToBlackBy( ledsStrip, NUM_LEDS_STRIP, 20);
  int pos = beatsin16(13,0,NUM_LEDS_STRIP);
  ledsStrip[pos] += CHSV( gHue, 255, 192);
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}

void sinelonSetup() {
  shouldClear = false;
  FastLED.setBrightness(128);
}

void noclearSetup() {
  shouldClear = false;
  FastLED.setBrightness(128);
}


void bpm()
{
  static uint8_t gHue = 0;
  uint8_t BeatsPerMinute = 100;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS_RING; i++) {
    ledsRing[i] = ColorFromPalette(palette, gHue+(i*4), beat-gHue+(i*10));
  }
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}

void juggle() {
  static uint8_t gHue = 0;
  fadeToBlackBy( ledsStrip, NUM_LEDS_STRIP, 20);
  fadeToBlackBy( ledsRing, NUM_LEDS_RING, 10);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    ledsStrip[beatsin16(i+7,0,NUM_LEDS_STRIP)] |= CHSV(dothue, 200, 255);
//    ledsRing[beatsin16(i+7,0,NUM_LEDS_RING)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  for( int i = 0; i < 4; i++) {
    ledsRing[beatsin16(i+3,0,NUM_LEDS_RING)] |= CHSV(dothue, 200, 255);
    dothue += 16;
  }
}

void flashySetup() {
  shouldClear = false;
  // FastLED.setBrightness(200);

}
void flashy() {
  static uint8_t gHue = 0;
  fadeToBlackBy( ledsStrip, NUM_LEDS_STRIP, 2);
  fadeToBlackBy( ledsRing, NUM_LEDS_RING, 2);

  EVERY_N_MILLISECONDS(200) { ledsRing[random( 0, NUM_LEDS_RING )] = CHSV(random(0, 255),240,128); }
  EVERY_N_MILLISECONDS(100) { ledsStrip[random( 0, NUM_LEDS_STRIP )] = CHSV(random(0, 255),240,128); }

  EVERY_N_SECONDS(60) { fill_solid(ledsStrip,NUM_LEDS_STRIP,CRGB::White); fill_solid(ledsRing,NUM_LEDS_RING,CRGB::White); }

}

void dotsSetup() {
  shouldClear = false;
  currentDelay = 30;

}
void dots() {
  static uint8_t gHue = 0;
  static uint8_t p1 = 0;
  static uint8_t p2 = 5;
  static uint8_t p3 = 10;
  fadeToBlackBy( ledsRing, NUM_LEDS_RING, 128/2);


  int b = beatsin16(30,0,100);

  p1 += uint8_t( float(b-50) / 30.0 );
  p2 += uint8_t( float(b-50) / 30.0 );
  p3 += uint8_t( float(b-50) / 30.0 );


  ledsRing[p1%16] = CHSV(190+gHue,240,128);
  ledsRing[p2%16] = CHSV(90+gHue*2,240,128);
  ledsRing[p3%16] = CHSV(10+gHue*3,240,128);

  EVERY_N_MILLISECONDS(30) {gHue++;}

  EVERY_N_SECONDS(2) {currentDelay++; if(currentDelay>200) currentDelay=10; }


}




void checkSerial() {
    if(Serial.available() > 0) {
        Serial.read();
        nextMode(1);
    }
}


void loop() {
    if(shouldClear)
      clear();
    modes[currentMode]();
    FastLED.show();

    EVERY_N_SECONDS(1) { checkSerial(); }
    EVERY_N_MILLISECONDS(500) { checkButton();  }
    FastLED.delay(currentDelay);
}
