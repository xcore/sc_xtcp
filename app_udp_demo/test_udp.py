#!/usr/bin/python
import socket,sys

# This simple script sends a UDP packet to port 15533 at the
# IP address given as the first argument to the script
# This is to test the simple UDP example XC program

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print "Connecting.."
sock.connect((sys.argv[1], 15533))
print "Connected"

msg = "hello world"
print "Sending message: " + msg
sock.send(msg)

print "Closing..."
sock.close()
print "Closed"
