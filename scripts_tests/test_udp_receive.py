import socket,sys,time,random,struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.connect((sys.argv[1], 100))

print "Connected"

msg = "Start"
sock.send(msg[0:])

time.sleep(100)

print "Done"
sock.close()
