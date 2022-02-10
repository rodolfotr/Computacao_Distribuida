import socket
import struct

GRUPO_MULTICAST = '230.4.5.6'
PORTA_MULTICAST = 6543

IP_PORTA_BACKUP = b'127.0.0.1:5000'

while True:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    sock.bind((GRUPO_MULTICAST, PORTA_MULTICAST))

    mreq = struct.pack("4sl", socket.inet_aton(GRUPO_MULTICAST), socket.INADDR_ANY)

    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    msg, cliente = sock.recvfrom(10240)
    cliente_ip = cliente[0]

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

    sock.sendto(IP_PORTA_BACKUP, (cliente_ip, 5005))
    print('sended', IP_PORTA_BACKUP, 'to', cliente_ip, '5005')

