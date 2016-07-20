require 'socket'
u1 = UDPSocket.new
u1.bind("127.0.0.1", 4913)
u1.send "abc", 0, "127.0.0.1", 4913

sock = UDPSocket.new
sock.connect("127.0.0.1", 4913)
puts sock.recvfrom(10)
sock.close