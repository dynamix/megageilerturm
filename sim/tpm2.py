import socket, select, struct
import opc
import time
import thread
import glob

# forward & tranlsate tmp2.net to our OPC simulator

frames = []
record = False

def tpm2():
	global frames
	pixel_data = None

	port= 0xFFE2

	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sock.bind(('127.0.0.1', port))

	client = opc.Client('localhost:7890')
	if client.can_connect():
	    print 'connected'
	else:
	    print 'cant connect'


	my = 30
	mx = 12
	max = mx*my
	last = 0
	data_size = 3 * 4 * 30

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
	    pixels = [ [ord(pixel_data[i*3]),ord(pixel_data[i*3+1]),ord(pixel_data[i*3+2])] for i in range(max)]

	    if record:
	    	frames.append(pixel_data)

	    if client.put_pixels(pixels, channel=0):
	        pass
	    else:
	        print 'not connected'
    # time.sleep(1/30.0)


thread.start_new_thread( tpm2 , ())
max_idx = 0
for fn in glob.glob('*.rec'):
	idx = int(str.split(fn,".")[0])
	if idx > max_idx:
		max_idx = idx
max_idx = max_idx + 1

while 1:
	raw_input("")
	if record == False:
		record = True
		frames = []
		print "Recording"
	else:
		fn = '%d.rec' % (max_idx)
		file = open(fn, 'wb')
		for frame in frames:
			file.write(frame)
		file.close
		print "Recorded", len(frames), "frames into", fn
		record = False
		max_idx = max_idx + 1
		frames = []

# Packet start byte	0x9C
# Frame size in 16 bits	High-Byte first, then
# Low-Byte
# Packet number	1-255
# Number of packets	1-255
# User data	1 - 1,490 bytes of data or command with parameters
# Packet end byte	0x36

