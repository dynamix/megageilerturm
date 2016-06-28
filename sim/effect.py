import opc
import time
import socket
import random
from colour import Color
import math

# we need one switch effect - maybe fade out fade in? :D

#  generic:
#  - one runing pixel around the global with a trail .. followed by a 3x3 pixel catching up - DONE
#  - two 3 or more pixels lines running in th oposit direction
#  -- if they intersect each other, blend the colors!
#  -- change speed over time?!
#  -- make pixels with different speeds
#  - falling rain from the top -> into a pond of water + fading
#  - random sparkling pixels
#  - exploding pixel like rockets :D -> requires a particle system -> we can do that on the teensy
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

# - with motion:
# - glow the sphere in tat the point where the acceleration vector hits the sphere


# recorded : pulsing plus mouse movement of multiple pixels


def idx(x,y):
    x = int(x) % 12
    y = int(y) % 30
    if x % 2 != 0:
        y = 29 - y

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



dx = random.random()
dy = random.random()
x = 0
y = 0


def sgn(f):
  if f > 0: return 1
  if f < 0: return -1
  return 0



def normal2d(v):
    return (-v[1],v[0])

def r2d(a):
    return a * (180/math.pi)

def d2r(a):
    return a / (180/math.pi)

def m(t,c,r,u,v):
    print t,r2d(t), math.cos(t),math.sin(t),c
    x = c[0] + (r * math.cos(t)) * u[0] + (r * math.sin(t)) *v[0]
    y = c[1] + (r * math.cos(t)) * u[1] + (r * math.sin(t)) *v[1]
    z = c[2] + (r * math.cos(t)) * u[2] + (r * math.sin(t)) *v[2]
    return [x,y,z]

def cross(a,b):
    x=a[1]*b[2]-a[2]*b[1]
    y=a[2]*b[0]-a[0]*b[2]
    z=a[0]*b[1]-a[1]*b[0]
    return (x,y,z)

def unit(v):
    l = math.sqrt(v[0]**2 + v[1]**2 + v[2]**2)
    return [ v[0] / l, v[1] / l, v[2] / l]

def plane(v,x,y,z,d):
    return v[0] * x + v[1] * y + v[2] * z - d

def onSphere(v,r):
    if r == 0:
        print "NOO",r
    #print "v", v,"r",r
    #print "theta",v[2]/r
    #print "phi",v[1]/v[0]
    theta = math.acos(v[2]/r)
    if v[0] == 0:
        phi = 0
    else:
        phi = math.atan(v[1]/v[0])

    return ( r2d(theta), r2d(phi) )
    #return ( theta, phi )

def spherical2(t, pixels):

    ledsPerStrip = 60
    strips = 6
    totalLeds = ledsPerStrip * strips
    sampleNum = 12

    o = t%30/10.0

    # just going from bottom to top -> speed is our normal vector too
    n = (0,1,0)
    p0 = (0,-1+n[1]*o,0)

    # c0 center of our sphere is (0,0,0)
    R = 1

    D = n[0] * p0[0] + n[1] * p0[1] + n[2] * p0[2]

    # p = (c0 - p0*n) / |n|
    length_n = math.sqrt(n[0]*n[0] +  n[1]*n[1] + n[2]*n[2])
    #print n[0] * p0[0], n[1] * p0[1],n[2] * p0[2],D
    p = (n[0] * p0[0] + n[1] * p0[1] + n[2] * p0[2]) / length_n
    # p = (n[0] * p0[0] + n[1] * p0[1] + n[2] * p0[2] - D) / length_n

    if p < -1 or p > 1:
        print "no intersect",p
        return

    #print "intersect", p

    # radius of our intersecting circle
    r = math.sqrt(R*R - p*p)
    #print p,r

    # center pointer of our intersecting circle
    c = ( p * n[0]/length_n, p * n[1]/length_n, p * n[2]/length_n )
    #print c



    # get one point
    # set y,z = 0
    # Cz  = D
    j = [0,0,0]

    if n[0] == 0:
        j[0] = D
    else:
        j[0] = D/n[0]

    # get y, set z = 0
    if n[1] == 0:
        j[1] = D
    else:
        j[1] = (D - n[0] * j[0]) / n[1]

    # get z
    if n[2] == 0:
        j[2] = D
    else:
        j[2] = D - ((n[0] * j[0] + n[1] * j[1])) / n[2]


    u = (j[0]-c[0],j[1]-c[1],j[2]-c[2])
    v = cross(u,n)

    # print c,unit(u),unit(v)

    m0 = m(0,c,r,unit(u),unit(v))

    # we want to sample for six degrees as this is the highest resulution we get
    # d = r * (180/math.pi)
    t = d2r(360/sampleNum)

    samples = [ m(i*t,c,r,unit(u),unit(v)) for i in range(sampleNum)]
    #print samples[0]

    #print onSphere(m0,r)

    # convert to spherical coordinates
    spherical_coords = [onSphere(v,r) for v in samples]


    #print samples[1],samples[30]
    p#rint spherical_coords[0], samples[0]


    # convert to LED
    # for i in range(60):
    #     lc = spherical_coords[i]
    #     x = int(lc[0] / 30)
    #     y = int( (90-lc[1]) / 6)
    #     if samples[i][1] > 0:
    #         a = 180 - lc[0]
    #         x = int(a / 30)
    #         y = 15 + int( (lc[1]) / 6)
    #     pixels[idx(x,y)] = [255,0,0]


    #for co in [0,1,2,3,4]:
    for co in range(sampleNum):

        #print  spherical_coords
        lc = spherical_coords[co]

        x = int(lc[0] / 30)
        #x = 1
        y = int( (90-lc[1]) / 6)
        #if samples[co][0] > 0:
        #    y = 29 - y
        if samples[co][1] > 0:
            a = 180 - lc[0]
            x = int(a / 30)
            y = 15 + int( (lc[1]) / 6)

        print co, lc, "leds->",x,y,samples[co]
        pixels[idx(x,y)] = [255,0,0]



def spherical(t, pixels):
    # for now 2d
    o = t%40/10.0
    v = (1,-1) # 45degree velocity
    p0 = (-1+v[0]*o,1+v[1]*o)

    nv = normal2d(v)
    # x2 = point
    # x1 = vel
    m = nv[0]/nv[1];
    c = p0[1]-m*p0[0];


    x1 = -1
    x2 = 1
    y1 = m * x1 + c
    y2 = m * x2 + c


    dx = x2 - x1
    dy = y2 - y1

    dr = math.sqrt(dx*dx+dy*dy)
    dr_2 = dx*dx+dy*dy
    D = x1 * y2 - x2 * y1
    intersect = (dx*dx+dy*dy)-(D*D)

    for i in range(30):
        pixels[idx(0,i)] = [0,0,255]
        pixels[idx(6,i)] = [0,0,200]


    if intersect < 0:
        print "no intersect", intersect
        return

    di = math.sqrt(dr_2 - D*D)
    ix1 = (D * dy + dx*sgn(dy) * di) / dr_2
    ix2 = (D * dy - dx*sgn(dy) * di) / dr_2

    iy1 = (-D * dx + math.fabs(dy) * di) / dr_2
    iy2 = (-D * dx - math.fabs(dy) * di) / dr_2



    # degrees = 90 - (math.asin(iy1/math.sqrt(ix1**2+iy1**2)) * (180/math.pi))
    d1 = math.asin(iy1) * (180/math.pi)
    degrees = d1 + 90
    led = int(degrees / 6.0)
    ledx = 0
    if ix1 > 0:
        ledx = ledx + 6
    pixels[idx(ledx, led)] = [255,0,0]

    #degrees2 =  90 + (math.asin(iy2/math.sqrt(ix2**2+iy2**2)) * (180/math.pi))
    d2 = math.asin(iy2) * (180/math.pi)
    degrees2 =  d2 + 90
    led2x = 0
    if ix2 > 0:
        led2x = led2x + 6

    led2 = int(degrees2 / 6.0)

    print degrees,"(",d1,")","->",led," ",degrees2,"(",d2,")","->",led2

    pixels[idx(led2x, led2)] = [0,255,0]


    #print ix1,iy1,ix2,iy2



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
    gx = gx + 0.1
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
    spherical2(t,pixels)
    if client.put_pixels(pixels, channel=0):
        pass
    else:
        print 'not connected'
    time.sleep(1/30.0)

