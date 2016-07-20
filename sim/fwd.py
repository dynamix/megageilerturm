import opc
import time
import socket

# forwards a Glediator recorded file to the simulator

def pix(b,i):
    # translate zigzag
    x = int(i / 30)
    if x % 2 == 0:
        y = 29 - (i%30)
    else:
        y = i % 30
    i = y*12+x
    o = i * 3
    return [ ord(b[o]) , ord(b[o+1]), ord(b[o+2]) ]

f = open("/Users/martin/Downloads/test.rec", "rb")


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

while True:
    b = f.read(360*3)

    pixels = [ pix(b,i) for i in range(360)]
    # p2 = []
    # for x in range(0,12):
    #     for y in range(0,30):
    #          if x % 2 == 0:
    #             y = y * 12
    #         else:
    #             y =

    #         p2.append(pixels[y+x])


    # for p in pixels:
    #     p[0] = p[0] - 5
    #     if p[0] < 0:
    #         p[0] = 255

    if client.put_pixels(pixels, channel=0):
        pass
    else:
        print 'not connected'
    time.sleep(1/30.0)


f.close