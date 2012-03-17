#!/usr/bin/env python
import sys
import socket
import select
import json
import math

#from pudb import set_trace; set_trace()

CRLF = '\r\n'
DEBUG = 1
class MalformedMessage(Exception): pass
class ConnectionClosed(Exception): pass
class UnknownServer(Exception): pass

class Manage:

    def __init__(self, host, port):
        self.host = host
        self.port = int(port)
        self.clients = []
        self.perfect_numbers = []
        self.highest_sent = 0
        self.highest_recvd = 0

    def start(self):
        backlog = 5
        listen = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listen.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listen.bind((self.host, self.port))
        listen.listen(backlog)
        theinput = [listen]
        try:
            while True:
                inputrdy, outputrdy, exceptrdy = select.select(theinput, [], [])
                for fd in inputrdy:
                    if fd == listen:
                        client, address = listen.accept()
                        theinput.append(client)
                        obj = self._socket_recv(client)
                        self._handle_input(client, address, obj)
                    elif fd in theinput:
                        obj = self._socket_recv(fd)
                        if obj: self._handle_input(client, address, obj)
                        #else: continue
        except (KeyboardInterrupt, ConnectionClosed):
            pass
        finally:
            print '\nexiting...'

    def _handle_input(self, client, address, obj):
        if obj['type'] == 'ack':
            # save server info and send range.
            self._add_client(address, obj)
            range_data = self._calc_range(obj['data']['perform'])
            self._socket_send(client, range_data)
        elif obj['type'] == 'result':
            # receive calculation data from compute and send a new range.
            client_obj = self._get_client(address, obj)
            if not client_obj:
                raise UnknownServer('Server %s has not sent Ack.' % address[1])
            self._save_result(obj['data'])
            range_data = self._calc_range(obj['data']['perform'])
            self._socket_send(client, range_data)
        elif obj['type'] == 'report':
             # if a request for a report is made, send all info
             report_data = self._format_report()
             self._socket_send(client, report_data)

    def _socket_send(self, sock, obj):
        data = json.dumps(obj)
        if DEBUG == 1: print 'outgoing: ', data
        sock.sendall('%s%s' % (data, CRLF))

    def _socket_recv(self, sock):
        try:
            data = sock.recv(4096)
        except:
           return False # do nothing!! 
        sizepos = data.find(CRLF)
        if sizepos == -1:
           return False 
        data = data[:sizepos]
        if DEBUG == 1: print 'incoming: ', data
        return json.loads(data)

    def _add_client(self, address, obj):
        if not self._get_client(address, obj):
            self.clients.append({'id': address[1], 'host': address[0], 
                    'perform': obj['data']['perform']})
            return True
        return False

    def _get_client(self, address, obj):
        for client in self.clients:
             if address[1] == client['id']:
                 return client 
        return False

    def _calc_range(self, perform):
        upper = math.floor(self.highest_sent + (perform * 4000))
        if upper <= sys.maxint:
            out = {'type': 'range', 'data': {'lower': self.highest_sent, 'upper': self.highest_sent}}
        out = {'type': 'range', 'data': {'lower': self.highest_sent, 'upper':  upper}}
        self.highest_sent = upper
        return out

    def _save_result(self, data):
        self.highest_recvd = data['upper']
        self.perfect_numbers.extend(data['perfect'])

    def _format_report(self):
        return {'type': 'result',
                'data': {'highest_recvd': self.highest_recvd,
                    'perfect_numbers': self.perfect_numbers,
                    'clients': self.clients }}

if __name__ == "__main__":
    if not len(sys.argv) == 3:
        print 'usage: manage host port'
        exit(1)
    server = Manage(sys.argv[1], sys.argv[2])
    server.start()
