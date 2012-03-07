#!/usr/bin/env python 

import socket
import sys
import select
import struct

def main(port):
    backlog = 5
    host = ''
    buff = 4096
    listen = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen.bind((host, port))
    listen.listen(backlog)
    print "server running"
    while 1: 
        client, address = listen.accept()
        data = client.recv(buff)
        if data:
            print struct.unpack("!B", data)
            client.send(data)
        client.close()

if __name__ == "__main__":
    main(8080)
