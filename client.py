#!/usr/bin/env python 

import socket 
import struct

host = '' 
port = 8080 
size = 4096
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
sock.connect((host,port)) 
data = struct.pack("!B", "word!",42) 
sock.send(data) 
data = sock.recv(size) 
sock.close() 
print 'Received:', data
