import socket,sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print "Connecting.."
sock.connect((sys.argv[1], 15533))
print "Connected"


for i in range(0,10):

    msg = "\0hello world "
    for j in range(i):
        msg = msg + str(j)

    msg = msg + '\0'
    msg = chr(len(msg)) + msg

    msg = msg + msg

    print "Sending message: " + msg[1:-1]
    sock.send(msg[0:2])
    sock.send(msg[2:])
    reply = sock.recv(14)
    print "Reply: " + str([ord(x) for x in list(reply)])

print "Closing..."
sock.close()
print "Closed"
