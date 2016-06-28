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

ser.write(b'X')

client = opc.Client('localhost:7890')
if client.can_connect():
    print 'connected'
else:
    print 'WARNING: could not connect'


while True:
	print ser.readline()
