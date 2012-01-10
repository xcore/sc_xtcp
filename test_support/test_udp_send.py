import socket,sys,time,random,struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.connect((sys.argv[1], 100))

print "Connected"

n = 0

for i in range(0,99999):
    msg = struct.pack('I', n)

    for j in range(0,300):
        msg = msg + 'xx'

    sock.send(msg[0:])

    n = n + 1
    #time.sleep(2.0)

print "Done"
sock.close()
