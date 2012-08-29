import socket,sys,time,random,struct

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], 101))

print "Connected"

msg = "Start"
sock.send(msg[0:])

while (True) :
    msg = sock.recv(4096)

print "Done"
sock.close()
