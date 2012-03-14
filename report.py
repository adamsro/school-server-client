import sys
import socket
import json

#from pudb import set_trace; set_trace()

class ConnectionClosed(Exception): pass
CRLF = '\r\n'

def socket_recv(sock):
    peekdata = sock.recv(2048, socket.MSG_PEEK)
    if peekdata == '':
        raise ConnectionClosed
    sizepos = peekdata.find(CRLF)
    if sizepos == -1:
        raise MalformedMessage('Did not find CRLF in message %r' % peekdata)
    data = read_exactly(sock, sizepos)
    return json.loads(data)

def read_exactly(sock, buflen):
    data = ''
    while len(data) != buflen:
        data += sock.recv(buflen - len(data))
    return data

def main(host, port):
    sock = socket.socket()
    sock.connect((host, int(port)))
    data = json.dumps({'type':'report'})
    sock.sendall("%s%s" % (data, CRLF))
    print json.dumps(socket_recv(sock), indent=4)

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2]);
