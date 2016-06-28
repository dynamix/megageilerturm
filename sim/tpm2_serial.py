import socket, select, struct
import opc
import time
import thread
import serial

# forward & tranlsate tmp2.net to the serial port


ser = serial.Serial('/dev/cu.usbmodem1411', 500000)

pixel_data = None

port= 0xFFE2

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('127.0.0.1', port))

my = 30
mx = 12
max = mx*my
last = 0
data_size = 3 * 4 * 30

def read_serial():
	while True:
		print ">",ser.readline()


thread.start_new_thread( read_serial , ())

ser.write(b'X')
#time.sleep(1)
# x = ['x' for i in range(382)]
# bps = 0
# t = time.time()
# while True:
# 	written = ser.write(x)
# 	bps = bps + written
# 	if time.time() - t > 1:
# 		print "written ", bps
# 		bps = 0
# 		t = time.time()
# 	#print "write", written


while True:
    data, addr = sock.recvfrom(2000)
    head,t,size,pn,seq = struct.unpack(">BBHBB", data[0:6])
    # print hex(head),hex(t),size,pn,seq
    if last + 1 != pn:
    	continue
    last = pn
    if pn == 1:
    	pixel_data = data[6:6+data_size]
    else:
    	pixel_data = pixel_data + data[6:6+data_size]
    if pn < seq:
    	continue
    last = 0
    ser.write(pixel_data[0:360])
    ser.write(pixel_data[360:720])
    ser.write(pixel_data[720:1080])
    #print "DONE: send"