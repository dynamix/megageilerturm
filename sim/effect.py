import opc
import time
import math
import socket
import random
from colour import Color

#  generic:
#  - one runing pixel around the global with a trail .. followed by a 3x3 pixel catching up
#  - falling rain from the top -> into a pond of water + fading
#  - random sparkling pixels
#  - exploding pixel like rockets :D
#  - lines runing around the circle .. maybe in oposit directions?!
#  - color wheels + rainbows
#  - knight riders
#  - two colors switches in fast like a police car
#  - slow and fast pulsing
#  - forward + sparkling waves
#  - scans - up down , left right mixing
#  - rollercoaster over the globe
#  - a rotating image or thing?!
#  - three direction changing snakes that are R&G&B and mix on meetup
#  - runing pixels /lineswith space inbeetween
#  - light on but a snake brither
#  - snake with a trail
#  - stuff that stacks .. comes from one direction and stays on
#  - rainbow with gliter
#  - confetti
# - super slow clor wheeled pulse
# - alternating segments of 2x2 + 4x2 edges

# - with motion:
# - glow the sphere in tat the point where the acceleration vector hits the sphere


# recorded : pulsing plus mouse movement of multiple pixels


def idx(x,y):
    x = int(x) % 12
    y = int(y) % 30
    # if x >= 6:
    #     x = (x - 6)*2
    #     y = 59 - y
    # else:
    #     x = x * 2
    return x*30+y

def round(x):
    return int(x+0.5)

def random_pixels(t,pixels):
    for x in range(10):
        i = int(random.random() * 359)
        pixels[i] = [0,0,255]

# only in the upper and lower part
def random_pixels_lower_upper(t,pixels):
    for i in range(10):
        x = random.randint(0,11)
        y = random.randint(0,6)
        if random.random() > 0.5:
            y = 30 - y
        pixels[idx(x,y)] = [0,0,255]


# super slow clor wheeled pulse
pulse = 0.0
spulse = 1
def m3(t, pixels):
    global pulse
    global spulse

    pulse = pulse + spulse * 0.01

    if pulse > 0.6:
        spulse = -1
    if pulse < 0.1:
        spulse = 1

    for x in range(0,12):
        for y in range(0,30):
            shift = 1 - (abs(15.0-y) / 15.0)
            l = min(max((pulse-shift),0),1)
            pixels[idx(x,y)] = [f * 255 for f in Color(hue=t/100.0 , saturation=1, luminance=l).rgb]

# a segment is formed of:
# X + + -
# + @ @ +
# + @ @ +
# - + + -

# X -> start, off
# @ -> full brightness
# + -> reduced brightness
# - -> off
def segments(t,pixels):
    # segment start (top left corner)
    bright = [255,0,0]
    dark = [50,0,0]
    sx = random.randint(0,12)
    sy = random.randint(0,26) # we need one space to the top and two to the bottom
    # row 0
    pixels[idx(sx+1, sy)] = dark
    pixels[idx(sx+2, sy)] = dark

    # row 1
    pixels[idx(sx,   sy+1)] = dark
    pixels[idx(sx+1, sy+1)] = bright
    pixels[idx(sx+2, sy+1)] = bright
    pixels[idx(sx+3, sy+1)] = dark

    # row 2
    pixels[idx(sx,   sy+2)] = dark
    pixels[idx(sx+1, sy+2)] = bright
    pixels[idx(sx+2, sy+2)] = bright
    pixels[idx(sx+3, sy+2)] = dark
   
    # row 3
    pixels[idx(sx+1, sy+3)] = dark
    pixels[idx(sx+2, sy+3)] = dark

def projected_circle(t,pixels):
    sx = random.randint(0,12)
    sy = random.randint(0,26) # we need one space to the top and two to the bottom

    bright = [255,0,0]
    radius = 0.1

    for x in range(0,12):
        for y in range(0,30):
            if spherical_distance(sx,sy, x,y) < radius:
                pixels[idx(x,y)] = bright

def spheric(x,y):
    theta = (x % 12) * math.pi * 2.0
    phi = (15-y % 30)* math.pi / 2.0
    return (theta, phi)

# https://en.wikipedia.org/wiki/Great-circle_distance
def spherical_distance(x1,y1, x2,y2):
    t1,p1 = spheric(x1,y1)
    t2,p2 = spheric(x2,y2)
    arg = math.sin((p2-p1/2.0))**2 + math.cos(p1) * math.cos(p2) * math.sin((t2-t1)/2.0)**2
    if arg < 0: 
        arg = -1 * arg
    sqrt = math.sqrt(arg)
    return 2.0 * math.asin(sqrt)


#  - one runing pixel around - followed by a 3x3 pixel catching up
def m1(t,pixels):
    global dx
    global dy
    global x
    global y
    if int(t) % 500 == 0:
        dx = random.random() / 2 * random.choice((1,-1))
        dy = random.random() / 2 * random.choice((1,-1))

    x = x + dx
    y = y + dy
    if x >= 12:
        x = 0.0
    if y >= 30:
        y = 0.0

    pixels[idx(x,y)] = [0,0,255]
    tx = 0
    ty = 0
    if dx > 0.5:
        tx = -1
    if dy > 0.5:
        ty = -1

    if tx != 0 or ty != 0:
        pixels[idx(x+tx,y+ty)] = [0,0,155]

    sx = -1 if dx > 0 else 1
    sy = -1 if dx > 0 else 1

    pixels[idx(x+sx*1,y+sy*1)] = [255,0,0]
    pixels[idx(x+sx*1,y+sy*2)] = [255,0,0]
    pixels[idx(x+sx*1,y+sy*3)] = [255,0,0]
    pixels[idx(x+sx*2,y+sy*1)] = [255,0,0]
    pixels[idx(x+sx*2,y+sy*2)] = [255,0,0]
    pixels[idx(x+sx*2,y+sy*3)] = [255,0,0]
    pixels[idx(x+sx*3,y+sy*1)] = [255,0,0]
    pixels[idx(x+sx*3,y+sy*2)] = [255,0,0]
    pixels[idx(x+sx*3,y+sy*3)] = [255,0,0]


#  - falling rain from the top
def m2(t, pixels):
    pass

def simplest_one_pixel_horizontal_runner(t,pixels):
    # on the real device we will have a ms timer
    xt = round(t * 4.0)
    # get runner position
    p = xt % 360
    pixels[p] = [255,0,0]

# one pixel which runs around - leaves a trail and is followed by a 3x3  one
def simple_horizontal_runner(t,pixels):
    # on the real device we will have a ms timer
    xt = round(t * 4.0)

    # get runner position
    p = xt % 360

    pixels = [ [0,0,0] for i in range(360)]

    pixels[p] = [255,0,0]
    pixels[(p-1)%360] = [180,0,0]
    pixels[(p-2)%360] = [80,0,0]


def simplest_one_pixel_vertical_runner(t,pixels):
    # on the real device we will have a ms timer
    xt = round(t * 2.0)

    y = int(xt / 12) % 30
    x = xt % 12

    p = idx(x,y)

    pixels[p] = [255,0,0]


gx = 0
gy = 0

def simplest_one_pixel_vertical_runner_with_globals(t,pixels):
    global gx
    global gy
    # on the real device we will have a ms timer
    gx = gx + 1
    if gx >= 12:
        gx = 0
        gy += 1
    if gy >= 30:
        gy = 0

    p = idx(gx,gy)
    pixels[p] = [255,0,0]


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
    pixels = [ [0,0,0] for i in range(360)]
    projected_circle(t,pixels)
    if client.put_pixels(pixels, channel=0):
        pass
    else:
        print 'not connected'
    time.sleep(1/1.0)

