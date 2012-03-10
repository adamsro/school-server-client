#!/usr/bin/env python

import sys
import socket
import select
import json

CRLF = '\r\n'
class MalformedMessage(Exception): pass
class ConnectionClosed(Exception): pass

def read_exactly(sock, buflen):
    data = ''
    while len(data) != buflen:
        data += sock.recv(buflen - len(data))
    return data

def peek(sock, buflen):
    data = sock.recv(buflen, socket.MSG_PEEK)
    return data

def socket_send(sock, obj):
    data = json.dumps(obj)
    size = len(data)
    sock.sendall('%i%s%s' % (size, CRLF, data))

def socket_recv(sock):
    peekdata = peek(sock, 1024)
    if peekdata == '':
        raise ConnectionClosed
    sizepos = peekdata.find(CRLF)
    if sizepos == -1:
        raise MalformedMessage('Did not find CRLF in message %r' % peekdata)
    sizedata = read_exactly(sock, sizepos)
    try:
        size = int(sizedata)
    except ValueError:
        raise MalformedMessage(
            'size data %r could not be converted to an int' % sizedata)
    data = read_exactly(sock, size)
    return json.loads(data)

def handle_data(obj):
    print repr(obj)

def main(host, port):
    backlog = 5
    buff = 4096
    listen = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen.bind((host, port))
    listen.listen(backlog)
    print "server running"
    client, address = listen.accept()
    try:
        while True:
            r_ok, address, address = select.select([client], [], [])
            for fd in r_ok:
                if fd == client:
                    print "rec obj!\n"
                    obj = socket_recv(client)
                    handle_data(obj)
                    print repr(obj)
    except (KeyboardInterrupt, ConnectionClosed):
        pass
    finally:
        print '\nexiting...'

if __name__ == "__main__":
    main('127.0.0.1', 8080)

