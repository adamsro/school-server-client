import sys
import socket
import json
import getopt

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

def main(host, port, kill):
    sock = socket.socket()
    sock.connect((host, int(port)))
    if kill:
        data = json.dumps({'type':'kill'})
    else:
        data = json.dumps({'type':'report'})
    sock.sendall("%s%s" % (data, CRLF))
    print json.dumps(socket_recv(sock), indent=4)


if __name__ == "__main__":
    try:
        opts, args = getopt.getopt(sys.argv[1:], "k", ["kill"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
    for o, a in opts:
        if o == "-k":
            main(sys.argv[2], sys.argv[3], True);
            exit(1)
    main(sys.argv[1], sys.argv[2], False);
