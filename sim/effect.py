import opc
import time
import socket
import random
from colour import Color

def pix(t,i):
    if random.random() < 0.05:
        return [ 0,0,200 ]
    return [ 0,0,0 ]

# rotator
def pix2(t,i):
    ring = int(t % 12)
    if i > ring * 30 and i < ring*30+30:
        return [f * 255 for f in Color(hue=0.3 + t, saturation=0.8, luminance=0.7).rgb]

    return [ 0,0,0 ]


ADDRESS = 'localhost:7890'
client = opc.Client('localhost:7890')
# Test if it can connect (optional)
if client.can_connect():
    print 'connected to %s' % ADDRESS
else:
    # We could exit here, but instead let's just print a warning
    # and then keep trying to send pixels in case the server
    # appears later
    print 'WARNING: could not connect to %s' % ADDRESS

start_time = time.time()


while True:
    t = time.time() - start_time
    pixels = [ pix(t,i) for i in range(360)]
    # p2 = []
    # for x in range(0,12):
    #     for y in range(0,30):
    #         p2.append(pixels[y*12+x])

    if client.put_pixels(pixels, channel=0):
        pass
    else:
        print 'not connected'
    time.sleep(1/30.0)

