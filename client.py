#!/usr/bin/env python 
import json
import socket 
import sys

CRLF = "\r\n"
host = '127.0.0.1'
port = 8082
size = 4096
out = ''
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
sock.connect((sys.argv[1],int(sys.argv[2]))) 

out = json.dumps({"type": "ack", "data": {"perform": 5.960024}})
print "sending ", out 
sock.send('%s%s' % (out, CRLF))

data = sock.recv(size) 
print "received: ", data

out = json.dumps({"type": "result", "data":
    {"upper": 59600, "perform": 5.96, "perfect": [0, 6, 28, 496, 8128]}})
print "sending ",out 
sock.send('%s%s' % (out, CRLF))

data = sock.recv(size) 
print "received: ", data
sock.close() 
