#!/usr/bin/env python
import sys
import socket
import select
import json

#from pudb import set_trace; set_trace()

CRLF = '\r\n'
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
        try:
            while True:
                client, address = listen.accept()
                r_ok, _ , _ = select.select([client], [], [])
                for fd in r_ok:
                    #if fd == client: # will this always be true?
                    obj = self._socket_recv(client)
                    self._handle_input(client, address, obj)
        except (KeyboardInterrupt, ConnectionClosed):
            pass
        finally:
            print '\nexiting...'

    def _handle_input(self, client, address, obj):
        print repr(obj)
        if obj['type'] == 'ack':
            # save server info and send range.
            self._add_client(address, obj)
            range_data = self._calc_range(obj['data'])
            print range_data
            self._socket_send(client, range_data)
        elif obj['type'] == 'result':
            # receive calculation data from compute and send a new range.
            if not self._is_client(address, obj):
                raise UnknownServer('Server %s has not sent Ack.' % address[1])
            self._save_result(address, data['data'])
            range_data = self._calc_range(obj['data'])
            print range_data
            self._socket_send(client, range_data)
        elif obj['type'] == 'report':
             # if a request for a report is made, send all info
             report_data = self._format_report(obj['data'])
             self._socket_send(client, range_data)

    def _socket_send(self, sock, obj):
        data = json.dumps(obj)
        sock.sendall('%s%s' % (data, CRLF))

    def _socket_recv(self, sock):
        peekdata = sock.recv(2048, socket.MSG_PEEK)
        if peekdata == '':
            raise ConnectionClosed
        sizepos = peekdata.find(CRLF)
        if sizepos == -1:
            raise MalformedMessage('Did not find CRLF in message %r' % peekdata)
        data = self._read_exactly(sock, sizepos)
        return json.loads(data)

    def _read_exactly(self, sock, buflen):
        data = ''
        while len(data) != buflen:
            data += sock.recv(buflen - len(data))
        return data
    
    def _add_client(self, address, obj):
        if self._is_client(address, obj):
            self.clients.append({'id': address[1], 
                            'host': address[0], 
                            'perform': obj['data']['result']})
            return True
        return False

    def _is_client(self, address, obj):
        found = False
        for x in range(len(self.clients)):
             if address[1] == self.clients['id']:
                 found = True
        if found == True:
            return True
        return False

    def _calc_range(self, data):
        upper = self.highest_sent + data['result'] * 1638
        if upper <= sys.maxint:
            return {'type': 'range', 'data': {'lower': self.highest_sent, 'upper': upper}}
        return {'type': 'range', 'data': {'lower': self.highest_sent, 'upper': self.highest_sent}}

    def _save_result(self, data):
        self.highest_recvd = data['range']['upper']
        self.perfect_numbers.extend(data['result'])

    def _format_report(self, data):
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
