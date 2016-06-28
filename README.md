# megageilerturm

Needs:

* https://github.com/zestyping/openpixelcontrol
* Arduino
* Flora
* https://github.com/adafruit/Adafruit_Sensor
* https://github.com/adafruit/Adafruit_LSM9DS0_Library
* https://github.com/FastLED/FastLED
* https://github.com/adafruit/Adafruit_BluefruitLE_nRF51
* https://github.com/adafruit/SD
* https://github.com/adafruit/Adafruit_TCS34725

Setup Emulation:

* needs Python (brew install python)
* Get & make https://github.com/zestyping/openpixelcontrol
* Run gl_server (bin/gl_server in openpixelcontrol) with sim/sphere.json (-l)
* Feed with sim/effect.py to test coded effects
* Feed with fwd.py (fwd reads a Glediator Recording)
* Feed directly from processings (get fadecandy and checkout the examples with OPC&processings https://github.com/scanlime/fadecandy)
* Feed directly from Glediator (set out matrix to 12x30, load patching.gled from sim/ as patching, set tpm2.net as output mode, open socket, start tpm2.py)


Sphere:

* 12 segements a 30 LEDS
* Segments are next to each other in a zigzag-way