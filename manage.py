#!/usr/bin/env python

import sys
import socket
import select
import json

#from pudb import set_trace; set_trace()

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
    sock.sendall('%s%s' % (data, CRLF))

def socket_recv(sock):
    peekdata = peek(sock, 1024)
    if peekdata == '':
        raise ConnectionClosed
    sizepos = peekdata.find(CRLF)
    if sizepos == -1:
        raise MalformedMessage('Did not find CRLF in message %r' % peekdata)
    data = read_exactly(sock, sizepos)
    return json.loads(data)

def calc_range(benchmark):
    return {'type': 'range', 'data': {'lower': 0, 'upper': 1024}}

def main(host, port):
    backlog = 5
    buff = 4096
    listen = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen.bind((host, port))
    listen.listen(backlog)
    print "server running"
    try:
        while True:
            client, _ = listen.accept()
            r_ok, _ , _ = select.select([client], [], [])
            for fd in r_ok:
                if fd == client:
                    obj = socket_recv(client)
                    print repr(obj)
                    if obj['type'] == 'performance':
                        range_data = calc_range(obj['data']['result'])
                        print range_data
                        socket_send(client, range_data)

    except (KeyboardInterrupt, ConnectionClosed):
        pass
    finally:
        print '\nexiting...'

if __name__ == "__main__":
    main('127.0.0.1', 8080)

