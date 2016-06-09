// we are going to use fast led as it provides some nice helper instead of the
// normal neopixel stuff
#include <FastLED.h>


// what we want to do here (pretty sure not everything will get done in time - only 1-2 would be totally ok :-)

// a) have a simple on and off switch (could just turn off the batteries)
// b) have a auto mode which cycles thorugh the things we came up with
// c) have a simple mode switch to cycle through our different modes
// d) be able to stream a previous captures pixel stream from the sd card
//    - we do have 360 pixels - that means we need 1080 bytes per frame - for 30 fps we need 32kb+/s read speed - sounds doable (we dont have room for buffering)
// e) be able to receive some control information via Bluetooth - this can be uses instead of the mode switch
// f) receive data from our microphone and i) influence the current animation or b) have a seperate audio controlled light mode
// g) receive some data from our color sensor and i) influence the current animation or b) have a seperate audio controlled light mode
// h) receive some data from our motion sensor i) influence the current animation or b) have a seperate audio controlled light mode

// planed setup
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


// with audio:
// - amped bars
// - snake that lights up the head driven by sound
// with movement:
// - translate stuff by some pixels up or down
// color sensor:
// - pick up color and just adjust

#define BUTTON_PIN  4
#define DATA_PIN    3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    360

CRGB leds[NUM_LEDS];

uint8_t button = 0;

void setup() {
	delay(3000); // if we fucked it up - great idea by fastled :D
	pinMode(BUTTON_PIN, INPUT);
}

int bs = 0;

void check_button() {
	bs = digitalRead(BUTTON_PIN);
	if (bs == HIGH) {
		button++;
	}
	Serial.print(button);
}

void loop() {
	// check if our main  button was pressed
	EVERY_N_SECONDS(1) { check_button(); }
}
